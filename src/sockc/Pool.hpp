
#pragma once

#include "predef.hpp"
#include <utility>
#include <boost/pool/pool.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "Config.hpp"

class Portal;

namespace csocks
{

class ChunkSizeNonExists:
    public std::exception
{
private:
    std::size_t chunkSize;

public:
    ChunkSizeNonExists(std::size_t _chunkSize):
        chunkSize(_chunkSize)
    {}

    virtual const char* what() const throw()
    {
        return ("chunk size [" + boost::lexical_cast<std::string>(chunkSize) + "] not inside pools").c_str();
    }
};

class Pool
{
    friend class Portal;
private:
    typedef boost::pool<boost::default_user_allocator_malloc_free> FixedPool;
    typedef boost::ptr_unordered_map<std::size_t, FixedPool> FixedPoolMap;
    FixedPoolMap pools;

public:
    char* malloc(std::size_t chunkSize)
    {
        return static_cast<char*>(getPool(chunkSize).malloc());
    }

    void free(std::size_t chunkSize, void* ptr)
    {
        getPool(chunkSize).free(ptr);
    }

private:
    FixedPool& getPool(std::size_t chunkSize) CS_ATTR_CONST
    {
        if (pools.begin()->first == chunkSize)
        {
            return *pools.begin()->second;
        }
        FixedPoolMap::iterator it = pools.find(chunkSize);
        if (CS_BLIKELY(it != pools.end()))
        {
            return *it->second;
        }
        else
        {
            throw ChunkSizeNonExists(chunkSize); // just throw while key non-exists.
        }
    }

    void initPools(const Config* config)
    {
        typedef boost::unordered_map<std::size_t, std::size_t> SizeMap;
        SizeMap sizeMap;
        std::size_t sizes[] = {
            config->drBufferSize,
            config->dwBufferSize,
            config->urBufferSize,
            config->uwBufferSize,
        };
        for (int i = 0; i < (sizeof(sizes) / sizeof(sizes[0])); ++i)
        {
            SizeMap::iterator it = sizeMap.find(sizes[i]);
            if (it == sizeMap.end())
            {
                sizeMap.insert(std::make_pair(sizes[i], static_cast<std::size_t>(1)));
            }
            else
            {
                it->second += 1;
            }
        }
        for (SizeMap::iterator it = sizeMap.begin(); it != sizeMap.end(); ++it)
        {
            addPool(it->first, config->initReserveBuffers * it->second);
        }
        CS_DUMP(pools.size());
    }

    void addPool(std::size_t chunkSize, std::size_t chunksReserve)
    {
        CS_SAY("adding Pool: {chunk-size:" << chunkSize << ", chunk-reserve:" << chunksReserve << "}");
        FixedPool* pool = new FixedPool(chunkSize, chunksReserve);
        pools.insert(chunkSize, pool);
    }
};

}
