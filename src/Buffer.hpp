
#pragma once

#include "predef.hpp"
#include <memory>

namespace csocks
{

class Buffer
{
public:
    char* data;
    std::size_t capacity;

    Buffer():
        data(NULL), capacity(0)
    {}

    explicit Buffer(std::size_t _size):
        data(NULL), capacity(_size)
    {
        setCapacity(capacity);
    }

    void setCapacity(std::size_t _capacity)
    {
        if (data != NULL)
        {
            delete data;
        }
        capacity = _capacity;
        data = new char[capacity];
    }

    ~Buffer()
    {
        delete data;
    }
};

}
