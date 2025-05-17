#include "functionnode.hpp"

namespace kvantum {

FunctionNode::FunctionIdentifier::FunctionIdentifier(FunctionCall* fcall)
    : FunctionIdentifier::FunctionIdentifier(fcall->var->id, fcall->arguments)
{
    if (fcall->var->isField()) {
        auto fa = fcall->var->as<FieldAccess*>();
        name = fa->base->as<Variable*>()->id;
        parentObj = fa->field->id;
    }
}

string FunctionNode::FunctionIdentifier::createName() const
{
    string name = parent != Type::get("Void") ? parent.getName() + "_" : "";
    name += this->name;
    for (auto& e : params)
        name += "_" + e->getName();
    return name;
}

bool FunctionNode::FunctionIdentifier::operator==(const FunctionIdentifier& other) const
{
    bool eq = this->name == other.name && this->params.size() == other.params.size();
    int i = 0;
    while (eq && i < this->params.size() && this->params[i] == other.params[i]) {
        i++;
    }
    return i == this->params.size() && eq;
}

} // namespace kvantum
