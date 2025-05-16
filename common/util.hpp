#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>
#include <exception>
#include <functional>
#include <stdint.h>
#include "common/diagnostics.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS_PLATFORM true
#else
#define WINDOWS_PLATFORM false
#endif

#define ITER_THROUGH(cont) cont.begin(),cont.end()

#define KVANTUM_ERROR nullptr
#define KVANTUM_SKIP nullptr
#define KVANTUM_VERIFY(cond, errMsg) \
    if (!(cond)) \
    kvantum::Diagnostics::error(errMsg)
#define KVANTUM_VERIFY_RETURN(cond, errMsg, err) \
    if (!(cond)) { \
        kvantum::Diagnostics::error(errMsg); \
        return err; \
    }
#define KVANTUM_VERIFY_ABANDON(cond,errMsg) KVANTUM_VERIFY_RETURN(cond,errMsg,;)
#define KVANTUM_VERIFY_ERROR(cond,errMsg) KVANTUM_VERIFY_RETURN(cond,errMsg,KVANTUM_ERROR)


#define panic(err) kvantum::Diagnostics::error(err)

#define CASE(v,do_smtg) case v: do_smtg; break;

using std::string;
using std::vector;

namespace kvantum {

template<typename Iterator, typename T, typename R>
vector<R> apply(Iterator begin, Iterator end, std::function<R(T)> func)
{
    vector<R> vec;
    while (begin != end) {
        vec.push_back(func(*begin++));
    }
    return vec;
}

template<typename Iterator>
static bool contains(Iterator begin,
                     Iterator end,
                     typename std::iterator_traits<Iterator>::value_type item)
{
    while (begin != end && *begin != item) {
        begin++;
    }
    return begin != end;
}

std::vector<string> std_string_split(string str);

} // namespace kvantum
