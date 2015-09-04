
#pragma once

#include "predef.hpp"
#include <ctime>
#include "Authority.hpp"

namespace csocks
{

class Authenticater
{
public:
    enum {
        CODE_OK,            // 成功
        CODE_EXPIRED,       // 会员过期
        CODE_TRAFFIC_EXHAUST, // 阶段流量耗尽
    };

    void restore(const Authority& authority)
    {
        // 持久化保存
    }

    template<typename Callback>
    void auth(const char* username, std::size_t usernameLen,
        const char* password, std::size_t passwordLen,
        const Callback& callback)
    {
        // send auth request
        Authority *authority = new Authority;
        callback(CODE_OK, authority);
    }
};

}
