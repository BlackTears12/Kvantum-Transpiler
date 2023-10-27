#include "interpreter/value.hpp"

namespace kvantum::interpreter
{
    IntValue* Value::asInt() { return static_cast<IntValue*>(this); }
    RatValue* Value::asRat() { return static_cast<RatValue*>(this); }
    StrValue* Value::asStr() { return static_cast<StrValue*>(this); }
    BoolValue* Value::asBool() { return static_cast<BoolValue*>(this); }
    ObjectValue* Value::asObj() { return static_cast<ObjectValue*>(this); }
    VoidValue* Value::asVoid() { return static_cast<VoidValue*>(this); }
    ArrayValue* Value::asArray() { return static_cast<ArrayValue*>(this); }
}