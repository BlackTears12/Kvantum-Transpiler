#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;

namespace c::ast
{
   struct Struct;
   class Type
   {
   public:
      Type(bool c){ constant = c; }
      virtual string getStr() = 0;
      virtual unsigned int getSize() = 0;
      virtual bool isPtr() { return false; }

   public:
      static Type* getVoid();
      static Type* getInt8();
      static Type* getInt16();
      static Type* getInt32();

      static Type* getUInt8();
      static Type* getUInt16();
      static Type* getUInt32();

      static Type* getFloat();
      static Type* getDouble();

      static Type* getPointer(Type* t);
      static Type* getStruct(Struct* s);
      
   protected:
      bool constant;
   };

   class Integer : public Type
   {
   public:
      Integer(uint8_t prec = 2,bool sign = true,bool constant = false) : Type(constant)
      {
         precision = prec;
         this->sign = sign;
      }

      string getStr() override 
      { 
          string tp;
          switch (precision)
          {
          case 1:
              tp = "char";
              break;
          case 2:
              tp = "int";
              break;
          default:
              tp = "long";
              break;
          }
          return string(constant ? "const " : "") + string(sign ? "" : "unsigned ") + tp; 
      }
      unsigned int getSize() override { return precision;  }
   private:
      uint8_t precision;
      bool sign;
   };

   class Float : public Type
   {
   public:
      Float(uint8_t prec,bool constant = false) : Type(constant)
      {
         precision = prec;
      }

   string getStr() override { return string(constant ? "const " : "") + (precision == 2 ? string("float") : string("double")); }
   unsigned int getSize() override { return precision; }
   private:
      uint8_t precision;
   };

   class Pointer : public Type
   {
   public:
      Pointer(Type* t = nullptr,bool constant = false) : Type(constant)
      {
         type = t;
      }
      string getStr() override { return string(constant ? "const " : "") + type->getStr() + "*"; }
      unsigned int getSize() override { return 2; }
      bool isPtr() { return true; }
      Type* getReferencedType() { return type; }
   private:
      Type* type;
   };


   class StructType : public Type
   {
   public:
       StructType(Struct* t) : Type(false){ s = t; }
       string getStr() override;
       unsigned int getSize() override;

       Struct* s;

       static std::map<Struct*, StructType*> mappedStructs;
   };

   class Void : public Type
   {
   public:
      Void() : Type(true){}
      string getStr() override { return "void"; }
      unsigned int getSize() override { return 0; }
   };
}