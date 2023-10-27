#pragma once
#include "ast/ast.hpp"
#include "common/module.hpp"
#include <stack>

using kvantum::Type;
using std::pair;
namespace kvantum::parser
{
   template <typename T>
   class SymbolStack
   {
   public:
      SymbolStack();
      bool isDeclared(string name);
      T get(string name);
      pair<string, T>& getNode(string name);

      void popSegment();
      void pushSegment(vector<pair<string,T>> vars);
      void push(pair<string,T> val);
   private:
      pair<string,T>* search(string name);

      vector<pair<string,T>> stack;
      pair<string,T>* lastFound;
      std::stack<uint16_t> segmentPtr;
   };

      template <typename T>
   SymbolStack<T>::SymbolStack()
   {
      lastFound = nullptr;
      segmentPtr.push(0);
   }

   template <typename T>
   bool SymbolStack<T>::isDeclared(string name)
   {
      return search(name) != nullptr;
   }

   template <typename T>
   T SymbolStack<T>::get(string name)
   {
      return search(name)->second;
   }
   template <typename T>
   pair<string, T>& SymbolStack<T>::getNode(string name)
   {
       return *search(name);
   }

   template <typename T>
   void SymbolStack<T>::popSegment()
   {
      uint16_t ptr = 0;
      if(!segmentPtr.empty()){
         ptr = segmentPtr.top();
         segmentPtr.pop();
      }
      lastFound = nullptr;
      stack.resize(ptr);
   }

   template <typename T>
   void SymbolStack<T>::pushSegment(vector<pair<string,T>> vars)
   {
      ///save the current stack size
      segmentPtr.push((unsigned short)stack.size());
      
      stack.insert(stack.end(),ITER_THROUGH(vars));
      lastFound = nullptr;
   }

   template <typename T>
   void SymbolStack<T>::push(pair<string,T> val)
   {
      stack.push_back(val);
   }

   template <typename T>
   pair<string,T>* SymbolStack<T>::search(string name)
   {
      if(lastFound != nullptr && lastFound->first == name)
         return lastFound;
      lastFound = nullptr;
      for(int i = (int)stack.size()-1;i >= 0;i--){
         if(stack[i].first == name){
            lastFound = &stack[i];
            return lastFound;
         }
      }
      return lastFound;
   }
}