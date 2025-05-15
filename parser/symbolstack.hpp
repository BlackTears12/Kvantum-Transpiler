#pragma once

#include "ast/ast.hpp"
#include "common/module.hpp"
#include <stack>

using kvantum::Type;
using std::pair;
namespace kvantum::parser
{
    template<typename T>
    class SymbolStack
    {
    public:
        SymbolStack();
        bool isDeclared(string name);
        bool isDeclaredLocal(string name);
        T get(string name);
        pair <string, T> &getNode(string name);

        void popSegment();
        void pushSegment(vector <pair<string, T>> vars);
        void push(pair <string, T> val);
    private:
        typename vector<pair<string,T>>::iterator search(string name);

        vector <pair<string, T>> stack;
        typename vector<pair<string,T>>::iterator lastFound;
        std::stack<uint16_t> segmentPtr;
    };

    template<typename T>
    SymbolStack<T>::SymbolStack()
    {
        lastFound = stack.end();
        segmentPtr.push(0);
    }

    template<typename T>
    bool SymbolStack<T>::isDeclared(string name)
    {
        return search(name) != stack.end();
    }

    template<typename T>
    bool SymbolStack<T>::isDeclaredLocal(std::string name)
    {
        auto loc = this->search(name);
        return loc != stack.end() && (std::distance(stack.begin(), loc) >= segmentPtr.top());
    }

    template<typename T>
    T SymbolStack<T>::get(string name)
    {
        return search(name)->second;
    }

    template<typename T>
    pair <string, T> &SymbolStack<T>::getNode(string name)
    {
        return *search(name);
    }

    template<typename T>
    void SymbolStack<T>::popSegment()
    {
        uint16_t ptr = 0;
        if (!segmentPtr.empty()) {
            ptr = segmentPtr.top();
            segmentPtr.pop();
        }
        stack.resize(ptr);
        lastFound = stack.end();
    }

    template<typename T>
    void SymbolStack<T>::pushSegment(vector<pair < string, T>> vars)
    {
        ///save the current stack size
        segmentPtr.push((unsigned short)stack.size());

        stack.insert(stack.end(), ITER_THROUGH(vars));
        lastFound = stack.end();
    }

template<typename T>
void SymbolStack<T>::push(pair<string, T> val)
{
    stack.push_back(val);
    lastFound = stack.end();
}

template<typename T>
typename vector<pair<string,T>>::iterator SymbolStack<T>::search(string name)
{
    if (lastFound != stack.end() && lastFound->first == name)
        return lastFound;
    lastFound = stack.end();
    if(stack.empty())
        return lastFound;
    for(auto iter = std::prev(stack.end());iter >= stack.begin();iter--){
        if (iter->first == name){
            lastFound = iter;
            return lastFound;
        }
    }
    return lastFound;
}
}