#include "common/type.hpp"
#include "ast/ast.hpp"
#include "common/compiler.hpp"
#include <iterator>
#include <optional>

namespace kvantum {

/*  Type methods  */

template<typename T>
std::optional<typename vector<T>::iterator> find(typename vector<T>::iterator begin,
                                                 typename vector<T>::iterator end,
                                                 T item)
{
    while (begin != end) {
        if (*begin == item)
            return begin;
        begin++;
    }
    return {};
}

void Type::initialize()
{
    PrimitiveType::initialize();
    ObjectType::initialize();
}

Type& Type::get(string name)
{
    std::vector<string> type_str = {"Int", "Float", "Bool", "Char", "Void"};
    auto tp = kvantum::find(type_str.begin(), type_str.end(), name);
    if (tp.has_value())
        return PrimitiveType::get(
            static_cast<PrimitiveType::TypeBase>(std::distance(type_str.begin(), tp.value())));
    throw std::invalid_argument("no type named " + name);
}

PrimitiveType& Type::asPrimitive()
{
    return static_cast<PrimitiveType&>(*this);
}
ObjectType& Type::asObject()
{
    return static_cast<ObjectType&>(*this);
}
ArrayType& Type::asArray()
{
    return static_cast<ArrayType&>(*this);
}
ReferenceType& Type::asReference()
{
    return static_cast<ReferenceType&>(*this);
}

/*Primitive Type methods*/

string PrimitiveType::getName() const
{
    string arr[] = {"Int", "Float", "Bool", "Char", "Void"};
    return arr[type];
}

void PrimitiveType::initialize()
{
    for (unsigned int i = 0; i < types.size(); i++)
        types[i] = std::make_unique<PrimitiveType>((TypeBase) i);
}

/*  ObjectType methods  */

bool ObjectType::equals(Type& other) const
{
    if (&getObject() == this)
        return true;
    return other.isObject() && other.asObject().getNode() == node;
}

unsigned int ObjectType::getAllocSize()
{
    unsigned int sz = 0;
    for (auto& e : node->fields) {
        if (e.second->isPrimitive())
            sz += e.second->getAllocSize();
        else
            sz += Type::getPointerAllocSize();
    }
    return sz;
}

bool ObjectType::hasFunction(Variable* var)
{
    if (parent && parent->hasFunction(var))
        return true;

    if (!var->isField())
        return node->methods.count(var->id) > 0;
    if (!hasField(var) || !node->fields.at(var->id)->isObject())
        return false;
    return node->fields[var->id]->asObject().hasFunction(var->as<FieldAccess*>()->field);
}

bool ObjectType::hasField(Variable* var)
{
    if (parent && parent->hasField(var))
        return true;

    if (!var->isField())
        return node->fields.count(var->id) > 0;
    if (!hasField(var) || !node->fields.at(var->id)->isObject())
        return false;
    return node->fields[var->id]->asObject().hasField(var->as<FieldAccess*>()->field);
}

Type& ObjectType::getFieldType(const string id)
{
    if (node->methods.count(id) || (parent && parent->asObject().node->methods.count(id)))
        return Type::get("Void");

    auto var = std::make_unique<Variable>(id);
    if (parent && parent->hasField(var.get()))
        return parent->getFieldType(var.get()->id);
    return *node->fields.find(id)->second;
}

FunctionNode* ObjectType::getFunction(Variable* const var)
{
    if (!var->isField()) {
        if (node->methods.count(var->id))
            return node->methods.at(var->id);
    } else if (hasField(var) && getFieldType(var->id).isObject()
               && getFieldType(var->id).asObject().hasFunction(var->as<FieldAccess*>()->field))
        return getFieldType(var->id).asObject().getFunction(var->as<FieldAccess*>()->field);
    return parent->getFunction(var);
}

void ObjectType::addFunction(string name, FunctionNode* fnode)
{
    if (fnode->hasTrait(FunctionNode::OVERRIDE)) {
        KVANTUM_VERIFY(parent && parent->hasFunction(name), "no inherited function named " + name);
        else KVANTUM_VERIFY(parent && parent->hasFunction(name)
                                && parent->getFunction(name)->hasTrait(FunctionNode::VIRTUAL),
                            "cannot override a non-virtual function " + name);
        else KVANTUM_VERIFY(parent && parent->hasFunction(name)
                                && (parent->getFunction(name)->getTraits() | FunctionNode::OVERRIDE)
                                       == (fnode->getTraits() | FunctionNode::VIRTUAL),
                            "function trait signature doesnt math with parent function");
    } else if (node->methods.count(name)) {
        panic(node->name + "already has method named " + name);
        return;
    }

    node->methods.insert({name, fnode});

    //if not static append the self ptr to arguments
    if (!fnode->hasTrait(FunctionNode::STATIC))
        fnode->formalParams.insert(fnode->formalParams.begin(), new Variable("self", *this));

    //for cctor we need to alloc memory and return self
    if (name == "new") {
        fnode->ast.insert(fnode->ast.begin(),
                          new Assigment(new Variable("self", *this),
                                        new DynamicAllocation(*this),
                                        true));
        fnode->ast.push_back(new Return(new Variable("self", *this)));
        fnode->setReturnType(*this);
    }
}

vector<std::pair<string, Type*>> ObjectType::getFields()
{
    auto fields = apply(ITER_THROUGH(node->fields),
                        std::function([](std::pair<string, Type*> p) { return p; }));
    if (parent) {
        auto parentF = parent->getFields();
        fields.insert(fields.begin(), parentF.begin(), parentF.end());
    }
    return fields;
}

vector<std::pair<string, FunctionNode*>> ObjectType::getMethods()
{
    auto methods = apply(ITER_THROUGH(node->methods),
                         std::function([](std::pair<string, FunctionNode*> p) { return p; }));
    if (parent) {
        auto parentM = parent->getMethods();
        methods.insert(methods.begin(), parentM.begin(), parentM.end());
    }
    return methods;
}

/* static methods  */

void ObjectType::initialize()
{
    object = std::make_unique<ObjectType>(new TypeNode("Object"));
}

/* ArrayType methods */

ArrayType& ArrayType::get(Type& itemT)
{
    auto iter = initiatedArrays.find(&itemT);
    if (iter != initiatedArrays.end())
        return *iter->second;

    ArrayType* arr = new ArrayType(itemT);
    initiatedArrays.emplace(&itemT, arr);
    return *arr;
}

/* ListType methods */
ListType::ListType(Type& t)
    : ObjectType(new TypeNode("[" + t.getName() + "]"))
    , type(t)
{
    auto accField = [this](string fieldname) {
        return new FieldAccess(new Variable("self"), new Variable(fieldname));
    };

    node->fields.emplace("_arr", &ArrayType::get(t));
    node->fields.emplace("size", &PrimitiveType::get(PrimitiveType::Integer));
    node->fields.emplace("max_size", &PrimitiveType::get(PrimitiveType::Integer));

    /*
            reSize Int -> Void
            resizes the old array to the specified size
            modifies max_value
       */
    FunctionNode* reSize = new FunctionNode(getTypeID() + "_reSize",
                                            Type::get("Void"),
                                            {new Variable("new_size", Type::get("Int"))});
    reSize->setName(getName() + "_reSize");
    reSize->ast = {/*
                arr = self._arr;
                self._arr = allocate[new_size*sizeof(type)];
                _c_builtin_::memcpy(self._arr,arr,self._size);
                self.max_size = new_size;
           */
                   new Assigment(new Variable("arr"), accField("_arr")),
                   new Assigment(accField("_arr"),
                                 new ArrayAllocation(type,
                                                     new Variable("new_size", Type::get("Int")))),
                   new FunctionCall(new Variable("memcpy"),
                                    {accField("_arr"), new Variable("arr"), accField("size")}),
                   new Assigment(accField("max_size"), new Variable("new_size", Type::get("Int")))};
    reSize->setTraitList(FunctionNode::PUBLIC);

    /*
            getSize -> Int
       */
    FunctionNode* getSize = new FunctionNode(getTypeID() + "_getSize", Type::get("Int"));
    reSize->setName(getName() + "_getSize");
    getSize->ast = {new Return(accField("size"))};
    getSize->setTraitList(FunctionNode::CONST | FunctionNode::PUBLIC);

    /*
            at Int -> T
       */
    FunctionNode* at = new FunctionNode(getTypeID() + "_at",
                                        type,
                                        {new Variable("index", Type::get("Int"))});
    at->setName(getName() + "_at");
    at->ast = {

    };

    /*
            append T -> Void
            appends the item to the list
            if max_size is reached resize the array
       */
    FunctionNode* append = new FunctionNode(getTypeID() + "_append",
                                            Type::get("Void"),
                                            {new Variable("item", type)});
    append->ast = {
        /*
                if self.max_size == self.size:
                    self.reSize(self.max_size*2);
                self.index(self.size+1) = item;
           */
        //new If_Else(new BinaryOperation(accField()))
    };
    append->setTraitList(FunctionNode::PUBLIC);

    /*
            Constructor
       */
    FunctionNode* cctor = new FunctionNode(getTypeID() + "_new",
                                           *this,
                                           {new Variable("initializer", ArrayType::get(t)),
                                            new Variable("arr_size", Type::get("Int"))});
    reSize->setName(getName() + "_new");
    cctor->ast = {
        /*
                self.size = 0;
                self.reSize(arr_size);
           */
        new Assigment(accField("size"), new Literal("0", Type::get("Int"))),
        new FunctionCall(accField("reSize"), {new Variable("arr_size")}, reSize),
    };
    cctor->setTraitList(FunctionNode::STATIC | FunctionNode::CONST | FunctionNode::PUBLIC);

    addFunction("getSize", getSize);
    addFunction("reSize", reSize);
    addFunction("new", cctor);
};

ListType& ListType::get(Type& t)
{
    auto iter = initiatedLists.find(&t);
    if (iter != initiatedLists.end())
        return *iter->second;
    ListType* lt = new ListType(t);
    initiatedLists.emplace(&t, lt);
    return *lt;
}

/* ReferenceType methods */
ReferenceType& ReferenceType::get(Type& t)
{
    auto iter = initatedReferences.find(&t);
    if (iter != initatedReferences.end())
        return *iter->second;
    ReferenceType* r = new ReferenceType(t);
    initatedReferences.emplace(&t, r);
    return *r;
}

std::array<unique_ptr<PrimitiveType>, PrimitiveType::Void + 1> PrimitiveType::types = {};
unique_ptr<ObjectType> ObjectType::object = {};
std::map<Type*, ArrayType*> ArrayType::initiatedArrays = {};
std::map<Type*, ListType*> ListType::initiatedLists = {};
std::map<Type*, ReferenceType*> ReferenceType::initatedReferences = {};

} // namespace kvantum
