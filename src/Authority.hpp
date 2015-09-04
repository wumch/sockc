
#pragma once

#include "predef.hpp"
#include <cryptopp/aes.h>
#include <ctime>

namespace csocks
{

class Authority
{
public:
    typedef int64_t traf_t;

    char key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    char iv[CryptoPP::AES::BLOCKSIZE];

    std::time_t expires;         // 会员过期时间 unix_timestamp
    uint32_t bandwidth;          // 带宽 byte
    traf_t traffic;              // 剩余流量 byte

    std::time_t traffic_expires; // 流量过期时间
    traf_t traffic_future;       // 下一阶段的剩余流量
    std::time_t traffic_expires_future; // 下一阶段流量过期时间

    std::size_t drBufSize, dwBufSize, urBufSize, uwBufSize;

    bool traffic_expired(std::time_t point) const
    {
        return traffic_expires <= point;
    }

    bool traffic_expired() const
    {
        return traffic_expired(std::time(NULL));
    }

    bool expired(std::time_t point) const
    {
        return expires <= point;
    }

    bool expired() const
    {
        return expired(std::time(NULL));
    }

    void traffic_forward()
    {
        traffic_expires = traffic_expires_future;
        traffic = traffic_future;
        traffic_future = 0;
    }

    bool allow(traf_t traf)
    {
        if (CS_UNLIKELY(traffic_expired()))
        {
            if (expired())
            {
                return false;
            }
            traffic_forward();
        }
        return traf <= traffic;
    }

    bool traf(traf_t traf)
    {
        if (CS_LIKELY(allow(traf)))
        {
            __sync_sub_and_fetch(&traffic, traf);
            return true;
        }
        else
        {
            return false;
        }
    }
};

}
