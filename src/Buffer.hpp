
#pragma once

#include "predef.hpp"
#include "Config.hpp"
#include "Pool.hpp"


namespace csocks
{

class Portal;

class Buffer
{
    friend class Portal;
private:
    static Pool pool;

public:
    std::size_t capacity;
    char* data;

    explicit Buffer(std::size_t _size):
        capacity(_size), data(pool.malloc(capacity))
    {}
};

}
