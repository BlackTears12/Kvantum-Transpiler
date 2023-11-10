#pragma once
#include "common/mainheader.hpp"
#include "common/type.hpp"

namespace kvantum::interpreter
{
   struct IntValue;
   struct RatValue;
   struct StrValue;
   struct BoolValue;
   struct ObjectValue;
   struct VoidValue;
   struct ArrayValue;

   struct Value
   {
      virtual bool isInt(){ return false; }
      virtual bool isRat(){ return false; }
      virtual bool isStr(){ return false; }
      virtual bool isBool(){ return false; }
      virtual bool isObj(){ return false; }
      virtual bool isVoid() { return false; }
      virtual bool isArray() { return false; }

      virtual IntValue* asInt();
      virtual RatValue* asRat();
      virtual StrValue* asStr();
      virtual BoolValue* asBool();
      virtual ObjectValue* asObj();
      virtual VoidValue* asVoid();
      virtual ArrayValue* asArray();

      virtual Value* add(Value* v) { throw std::invalid_argument("cannot add"); }
      virtual Value* sub(Value* v) { throw std::invalid_argument("cannot sub"); }
      virtual Value* mul(Value* v) { throw std::invalid_argument("cannot mul"); }
      virtual Value* div(Value* v) { throw std::invalid_argument("cannot div"); }

      virtual IntValue* convertToInt(Value* v) { throw std::invalid_argument("cannot convert"); }
      virtual RatValue* convertToRat(Value* v) { throw std::invalid_argument("cannot convert"); }
      virtual StrValue* convertToStr(Value* v) { throw std::invalid_argument("cannot convert"); }
      virtual BoolValue* convertToBool(Value* v) { throw std::invalid_argument("cannot convert"); }
      virtual VoidValue* convertToVoid(Value* v) { throw std::invalid_argument("cannot convert"); }
      virtual ObjectValue* convertToObj(Value* v) { throw std::invalid_argument("cannot convert"); }

      virtual int compare(Value* v) { throw std::invalid_argument("cannot compare"); }
   };

   struct IntValue : public Value
   {
   public:
      IntValue(int v) : value(v){}
      bool isInt(){ return true; }
      virtual Value* add(Value* v) { return new IntValue(value + v->asInt()->value); }
      virtual Value* sub(Value* v) { return new IntValue(value - v->asInt()->value); }
      virtual Value* mul(Value* v) { return new IntValue(value * v->asInt()->value); }
      virtual Value* div(Value* v) { return new IntValue(value / v->asInt()->value); }
      

      virtual int compare(Value* v) 
      {
         if(v->asInt()->value < value)
            return 1;
         if(v->asInt()->value > value)
            return -1;
         return 0;
      }

      int value;
   };

   struct RatValue : public Value
   {
      RatValue(double v) : value(v){}
      virtual Value* add(Value* v) { return new RatValue(value + v->asRat()->value); }
      virtual Value* sub(Value* v) { return new RatValue(value - v->asRat()->value); }
      virtual Value* mul(Value* v) { return new RatValue(value * v->asRat()->value); }
      virtual Value* div(Value* v) { return new RatValue(value / v->asRat()->value); }
         
      virtual int compare(Value* v) 
      {
         if(v->asRat()->value < value)
            return 1;
         if(v->asRat()->value > value)
            return -1;
         return 0;
      }  

      double value;
   };

   struct StrValue : public Value
   {
      StrValue(string v) : value(v){}
      virtual Value* add(Value* v) { return new StrValue(value + v->asStr()->value); }

      virtual int compare(Value* v) 
      {
         if(v->asStr()->value.size() < value.size())
            return 1;
         if(v->asStr()->value.size() > value.size())
            return -1;
         return 0;
      }  

      string value;
   };

   struct BoolValue : public Value
   {
      BoolValue(bool v) : value(v){}
      virtual Value* add(Value* v) { return new BoolValue(value || v->asBool()->value); }
      virtual Value* mul(Value* v) { return new BoolValue(value && v->asBool()->value); }
      bool invert() { return new BoolValue(!value); }

      virtual int compare(Value* v) 
      {
         return !v->asBool()->value == value;
      }  


      bool value;
   };

   struct ObjectValue : public Value
   {
      ObjectValue(ObjectType* t){ type = t; }

      virtual int compare(Value* v){ return 0; }

      ObjectType* type;
   };

   struct ArrayValue : public Value
   {
       ArrayValue(vector<Value*> *vals) :value_ptr(vals){}

       Value* index(int ind)
       { 
           if (value_ptr->size() <= ind)
               throw std::invalid_argument("index out of range for array");
           return (*value_ptr)[ind];
       }
       vector<Value*>* value_ptr;
   };

   struct VoidValue : public Value 
   {
   };
}