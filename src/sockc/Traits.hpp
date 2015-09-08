
#include "predef.hpp"
#include <ctime>

#pragma once

namespace csocks
{

template<int Platform>
class TraitsImpl
{};

template<>
class TraitsImpl<_CSOCKS_LINUX>
{
public:
    typedef struct timeval SocketTimeoutVal;
};

template<>
class TraitsImpl<_CSOCKS_WINDOWS_PC>
{
public:
    typedef size_t SocketTimeoutVal;
};

typedef TraitsImpl<_CSOCKS_PLATFORM> Traits;

}
