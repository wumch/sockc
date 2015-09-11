
#include "predef.hpp"
#include <ctime>

#pragma once

namespace csocks
{

template<int Platform>
class TraitsImpl
{};

template<>
class TraitsImpl<_CSOCKS_POSIX>
{
public:
    typedef struct timeval SocketTimeoutVal;
};

template<>
class TraitsImpl<_CSOCKS_LINUX>:
    public TraitsImpl<_CSOCKS_POSIX>
{};

template<>
class TraitsImpl<_CSOCKS_OSX>:
    public TraitsImpl<_CSOCKS_POSIX>
{};

template<>
class TraitsImpl<_CSOCKS_ANDROID>:
    public TraitsImpl<_CSOCKS_POSIX>
{};

template<>
class TraitsImpl<_CSOCKS_IOS>:
    public TraitsImpl<_CSOCKS_POSIX>
{};

template<>
class TraitsImpl<_CSOCKS_WIN>
{
public:
    typedef size_t SocketTimeoutVal;
};

template<>
class TraitsImpl<_CSOCKS_WINDOWS_PC>:
    public TraitsImpl<_CSOCKS_WIN>
{};

template<>
class TraitsImpl<_CSOCKS_WINDOWS_PHONE>:
    public TraitsImpl<_CSOCKS_WIN>
{};

typedef TraitsImpl<_CSOCKS_PLATFORM> Traits;

}
