#include "c_codegen/c_type.hpp"
#include "c_codegen/c_ast.hpp"

namespace c::ast
{
   Type* Type::getVoid(){ return new Void(); }
   Type* Type::getInt8(){ return new Integer(1); }
   Type* Type::getInt16(){ return new Integer(2); }
   Type* Type::getInt32(){ return new Integer(4); }
   Type* Type::getUInt8(){ return new Integer(1,false); }
   Type* Type::getUInt16(){ return new Integer(2,false); }
   Type* Type::getUInt32(){ return new Integer(4,false); }
   Type* Type::getFloat(){ return new Float(1); }
   Type* Type::getDouble(){ return new Float(2); }
   Type* Type::getPointer(Type* t){ return new Pointer(t); }
   Type* Type::getStruct(Struct* s)
   {
	   if(StructType::mappedStructs.count(s))
		   return StructType::mappedStructs[s];
	   auto st = new StructType(s);
	   StructType::mappedStructs.insert({s,st});
	   return st;
   }

   string StructType::getStr() { return s->getHeader(); }
   unsigned int StructType::getSize() 
   {
	   unsigned int sum = 0;
	   for (auto& e : s->fields)
		   sum += e->type->getSize();
	   return sum;
   }

   std::map<Struct*, StructType*> StructType::mappedStructs = {};

}