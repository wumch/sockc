
#pragma once

#include "predef.hpp"
#include <netinet/in.h>
#include <cstring>      // for memcpy
#include <vector>
#include <string>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include "stage/backtrace.hpp"
#include "Authenticater.hpp"
#include "Config.hpp"
#include "Crypto.hpp"
#include "Outlet.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

#define SINGLE_BYTE __attribute__((aligned(1), packed))

#define KICK_IF(err) if (CS_UNLIKELY(err)) { return; }

#define FALSE_IF(err) if (CS_UNLIKELY(ec)) { return false; }

namespace csocks
{

class Channel:
    public boost::enable_shared_from_this<Channel>
{
private:
    enum {
        PROTOCOL_V4 = 0x04,
        PROTOCOL_V5 = 0x05,
        PROTOCOL_VERSION = PROTOCOL_V5,
    };
    enum {
        CMD_CONNECT     = 0x01,
        CMD_BIND        = 0x02,
        CMD_UDP_ASSOC   = 0x03,
    } SINGLE_BYTE;

    // 认证方法
    enum {
        AUTH_NONE           = 0x00,        // 不认证
        AUTH_USERPASS       = 0x02,        // 用户名+密码
        AUTH_UNACCEPTABLE   = 0xff,        // 无可接受方法
    } SINGLE_BYTE;

    // 认证结果
    enum {
        AUTH_RES_SUCCESS    = 0x00,        // 成功
        AUTH_RES_FAILED     = 0x01,        // 失败
    } SINGLE_BYTE;

    // addr type
    enum AddrType {
        ADDR_IPV4 = 0x01,
        ADDR_DOMAIN = 0x03,
        ADDR_IPV6 = 0x04,
    } SINGLE_BYTE;

    enum {
        CONNECT_SUCCEED = 0x00,     // 连接成功
    } SINGLE_BYTE;

    enum {
        SOCKS5_CONNECT_SUCCEED                          = 0x00,
        SOCKS5_GENERAL_SOCKS_SERVER_FAILURE             = 0x01,
        SOCKS5_CONNECTION_NOT_ALLOWED_BY_RULESET        = 0x02,
        SOCKS5_NETWORK_UNREACHABLE                      = 0x03,
        SOCKS5_HOST_UNREACHABLE                         = 0x04,
        SOCKS5_CONNECTION_REFUSED                       = 0x05,
        SOCKS5_TTL_EXPIRED                              = 0x06,
        SOCKS5_COMMAND_NOT_SUPPORTED                    = 0x07,
        SOCKS5_ADDRESS_TYPE_UNSUPPORTED                 = 0x08,
        // defined by csocks:
        SOCKS5_DOMAIN_RESOLVE_FAILED                    = 0x10,
    } SINGLE_BYTE;

    enum {
        SOCKS4_CONNECT_SUCCEED = 0x00,
        SOCKS4_REQUEST_GRANTED = 90,
        SOCKS4_REQUEST_REJECTED_OR_FAILED,
        SOCKS4_CONNECT_FAILED,
        SOCKS4_REQUEST_REJECTED_USER_NO_ALLOW,
    } SINGLE_BYTE;

    // 认证方法 最大数量
    static const int maxNumMethods = 8;

    const Config* const config;

    UserOutletMap& users;

    asio::io_service& ioService;
    tcp::resolver resolver;

    tcp::socket ds, us;
    udp::socket dsu;
    Buffer bufdr, bufdw, bufur, bufuw;
    uint8_t dsVersion;  // 客户端协议版本

    Crypto crypto;
    Authenticater& authenticater;
    Authority* authority;

public:
    Channel(UserOutletMap& _users, asio::io_service& _ioService, Authenticater& _authenticater):
        config(Config::instance()), users(_users),
        ioService(_ioService), resolver(ioService),
        ds(ioService), us(ioService), dsu(ioService),
        bufdr(config->drBufferSize), bufdw(config->dwBufferSize),
        bufur(config->urBufferSize), bufuw(config->uwBufferSize),
        dsVersion(0),
        authenticater(_authenticater), authority(NULL)
    {
        setsockopt(ds.native(), SOL_SOCKET, SO_RCVTIMEO, &config->dsRecvTimeout, sizeof(config->dsRecvTimeout));
        setsockopt(ds.native(), SOL_SOCKET, SO_SNDTIMEO, &config->dsSendTimeout, sizeof(config->dsSendTimeout));
        setsockopt(us.native(), SOL_SOCKET, SO_RCVTIMEO, &config->usRecvTimeout, sizeof(config->usRecvTimeout));
        setsockopt(us.native(), SOL_SOCKET, SO_SNDTIMEO, &config->usSendTimeout, sizeof(config->usSendTimeout));
    }

    tcp::socket& downstream()
    {
        return ds;
    }

    void start()
    {
        KICK_IF(!prepare());
        readNumMethods();
    }


    ~Channel()
    {
        CS_SAY("channel [" << (std::size_t)this << "] destructing");
        shutdown();
    }

private:
    void readNumMethods()
    {
        CS_SAY("reading num-methods");
        // read
        //  +----+----------+----------+
        //  |VER | NMETHODS | METHODS  |
        //  +----+----------+----------+
        //  | 1  |    1     | 1 to 255 |
        //  +----+----------+----------+
        //  [               ]
        std::memset(bufdr.data, 0, bufdr.capacity);
        CS_DUMP(bufdr.capacity);
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity),
            asio::transfer_exactly(2),
            boost::bind(&Channel::handleGreet, shared_from_this(), asio::placeholders::error,
                asio::placeholders::bytes_transferred));
    }

    void handleGreet(const boost::system::error_code& err, int bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)
        KICK_IF(bytesRead != 2)

        char header[2];
        crypto.decrypt(bufdr.data, 2, header);
        dsVersion = header[0];
        CS_DUMP((int)dsVersion);
        CS_DUMP((int)header[1]);
        KICK_IF(dsVersion != PROTOCOL_V5 && dsVersion != PROTOCOL_V4);
        KICK_IF(header[0] != PROTOCOL_VERSION || header[1] < 1 || maxNumMethods < header[1]);

        // read
        //  +----+----------+----------+
        //  |VER | NMETHODS | METHODS  |
        //  +----+----------+----------+
        //  | 1  |    1     | 1 to 255 |
        //  +----+----------+----------+
        //                  [          ]
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity), asio::transfer_exactly(header[1]),
            boost::bind(&Channel::handleMethods, shared_from_this(), header[1],
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleMethods(int numMethods, const boost::system::error_code& err, int bytesRead)
    {
        CS_DUMP(numMethods);
        KICK_IF(err)
        KICK_IF(bytesRead != numMethods)

        char methods[maxNumMethods];
        crypto.decrypt(bufdr.data, numMethods, methods);

        uint8_t method;
        if (CS_BLIKELY(methods[0] == AUTH_USERPASS || (numMethods > 1 && methods[1] == AUTH_USERPASS)))
        {
            method = AUTH_USERPASS;
        }
        else
        {
            method = AUTH_UNACCEPTABLE;
            for (int i = 0; i < numMethods; ++i)
            {
                if (methods[i] == AUTH_USERPASS)
                {
                    method = methods[i];
                    break;
                }
            }
        }
        CS_DUMP((int)method);

        // write
        //  +----+--------+
        //  |VER | METHOD |
        //  +----+--------+
        //  | 1  |   1    |
        //  +----+--------+
        //  [             ]
        uint8_t data[2] = {PROTOCOL_VERSION, method};
        crypto.encrypt(data, 2, bufdw.data);

        if (CS_BUNLIKELY(method == AUTH_UNACCEPTABLE))
        {
            asio::async_write(ds, asio::buffer(bufdw.data, bufdw.capacity), asio::transfer_exactly(2),
                boost::bind(&Channel::shutdown, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else
        {
            asio::async_write(ds, asio::buffer(bufdw.data, bufdw.capacity), asio::transfer_exactly(2),
                boost::bind(&Channel::handleAuthSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
    }

    void handleAuthSent(boost::system::error_code err, int bytesSent)
    {
        KICK_IF(err)

        // read
        //  +----+------+----------+------+----------+
        //  |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
        //  +----+------+----------+------+----------+
        //  | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
        //  +----+------+----------+------+----------+
        //  [           ]
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity), asio::transfer_exactly(2),
            boost::bind(&Channel::handleUserLen, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUserLen(const boost::system::error_code& err, int bytesRead)
    {
        KICK_IF(err)

        char header[2];
        crypto.decrypt(bufdr.data, 2, header);
        KICK_IF(header[1] < 1)

        // read
        //  +----+------+----------+------+----------+
        //  |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
        //  +----+------+----------+------+----------+
        //  | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
        //  +----+------+----------+------+----------+
        //              [                 ]
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity), asio::transfer_exactly(header[1] + 1),
            boost::bind(&Channel::handleUserPassLen, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUserPassLen(const boost::system::error_code& err, int bytesRead)
    {
        KICK_IF(err)

        char* userPassLen = bufdw.data;         // now bufdw is idle, reuse it.
        crypto.decrypt(bufdr.data, bytesRead, userPassLen);
        uint8_t passLen = userPassLen[bytesRead - 1];
        KICK_IF(passLen < 1)
        userPassLen[bytesRead - 1] = 0x00;

        // read
        //  +----+------+----------+------+----------+
        //  |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
        //  +----+------+----------+------+----------+
        //  | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
        //  +----+------+----------+------+----------+
        //                                [          ]
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity), asio::transfer_exactly(passLen),
            boost::bind(&Channel::handleUserPass, shared_from_this(), userPassLen, bytesRead - 1,
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUserPass(const char* username, std::size_t usernameLen,
        const boost::system::error_code& err, int bytesRead)
    {
        KICK_IF(err)

        char* password = bufdw.data + usernameLen + 1;    // now bufdw is idle, reuse it.
        crypto.decrypt(bufdr.data, bytesRead, password);
        password[bytesRead] = 0x00;
        CS_DUMP(username);
        CS_DUMP(password);

        authenticater.auth(username, usernameLen, password, bytesRead,
            boost::bind(&Channel::handleAuthed, shared_from_this(), _1, _2, username, usernameLen));
//        authenticater.auth(username, usernameLen, password, bytesRead);
    }

    void handleAuthed(int code, Authority* _authority,
        const char* username, std::size_t usernameLen)
    {
        if (CS_BLIKELY(code == Authenticater::CODE_OK))
        {
            CS_DUMP("auth succeed");
            addToOutlet(std::string(username, usernameLen));
            authority = _authority;
            buildStage();

            crypto.setDecKeyWithIv(authority->key, sizeof(authority->key),
                    authority->iv, sizeof(authority->iv));

            // 认证成功
            // write
            //  +----+--------+-----+----+
            //  |VER | STATUS | KEY | IV |
            //  +----+--------+----------+
            //  | 1  |    1   | 16  | 16 |
            //  +----+--------+-----+----+
            //  [                        ]
            uint8_t data[2 + sizeof(authority->key) + sizeof(authority->iv)] = {PROTOCOL_VERSION, AUTH_RES_SUCCESS};
/* TODO: comment whilte test only
//            std::memcpy(data + 2, authority->key, sizeof(authority->key));
//            std::memcpy(data + (2 + sizeof(authority->key)), authority->iv, sizeof(authority->iv));
//            crypto.encrypt(data, sizeof(data), bufdw.data);

//            asio::async_write(ds, asio::buffer(bufdw.data, sizeof(data)), asio::transfer_exactly(sizeof(data)),
//                boost::bind(&Channel::handleAuthedSent, shared_from_this(),
//                    asio::placeholders::error, asio::placeholders::bytes_transferred));
// */

            crypto.encrypt(data, 2, bufdw.data);
            CS_DUMP((int)bufdw.data[0]);
            CS_DUMP((int)bufdw.data[1]);
            asio::async_write(ds, asio::buffer(bufdw.data, 2), asio::transfer_exactly(2),
                boost::bind(&Channel::handleAuthedSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else
        {
            CS_DUMP("auth failed");
            // 认证失败
            // write
            //  +----+--------+
            //  |VER | STATUS |
            //  +----+--------+
            //  | 1  |    1   |
            //  +----+--------+
            //  [             ]
            uint8_t data[2] = {PROTOCOL_VERSION, AUTH_RES_FAILED};
            crypto.encrypt(data, 2, bufdw.data);        // NOTE: username is dirty.

            asio::async_write(ds, asio::buffer(bufdw.data, 2), asio::transfer_exactly(2),
                boost::bind(&Channel::shutdown, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
    }

    void handleAuthedSent(const boost::system::error_code& err, int bytesRead)
    {
        CS_DUMP("auth-succeed packet sent");
        KICK_IF(err)
        // read
        //  +----+-----+-------+------+----------+----------+
        //  |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        //  +----+-----+-------+------+----------+----------+
        //  | 1  |  1  | X'00' |  1   | Variable |    2     |
        //  +----+-----+-------+------+----------+----------+
        //  [                          +1]
        asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity), asio::transfer_exactly(5),
            boost::bind(&Channel::handleCmd, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleCmd(const boost::system::error_code &err, std::size_t bytesRead)
    {
        CS_DUMP("received command");
        KICK_IF(err);

        char header[5];
        crypto.decrypt(bufdr.data, bytesRead, header);
        CS_DUMP((int)header[1]);
        CS_DUMP((int)header[3]);

        if (CS_BLIKELY(header[1] == CMD_CONNECT))
        {
            switch (header[3])
            {
            case ADDR_DOMAIN:
                asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity),
                    asio::transfer_exactly(header[4] + 2),
                    boost::bind(&Channel::handleDomainRequest, shared_from_this(),
                        asio::placeholders::error, asio::placeholders::bytes_transferred));
                break;
            case ADDR_IPV4:
                asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity),
                    asio::transfer_exactly(3 + 2),
                    boost::bind(&Channel::handleIpv4Request, shared_from_this(), header[4],
                        asio::placeholders::error, asio::placeholders::bytes_transferred));
                break;
            case ADDR_IPV6:
                // TODO: IPv6 support
                delete this;
                return;
//                asio::async_read(ds, asio::buffer(bufdr.data, bufdr.capacity),
//                    asio::transfer_exactly(5 + 2),
//                    boost::bind(&Channel::handleDomainRequest, shared_from_this(), header[4],
//                        asio::placeholders::error, asio::placeholders::bytes_transferred));
                break;
            default:
                dealConnectFailed(SOCKS5_ADDRESS_TYPE_UNSUPPORTED);
                break;
            }
        }
        else
        {
            // TODO: implement
            switch (header[1])
            {
            case CMD_BIND:
                break;

            case CMD_UDP_ASSOC:
                break;

            default:
                delete this;
                break;
            }
        }
    }

    void handleIpv4Request(char firstByte, const boost::system::error_code &err, std::size_t bytesRead)
    {
        KICK_IF(err)

        char ipPort[8] __attribute((aligned(4)));        // to align with 4 bytes with non-GNUC.
        crypto.decrypt(bufdr.data, 5, ipPort + 1);
        ipPort[0] = firstByte;
        uint32_t ip = ntohl(*reinterpret_cast<uint32_t*>(ipPort));
        uint16_t port = ntohs(*reinterpret_cast<uint16_t*>(ipPort + 4));
        KICK_IF(ip == 0 || port == 0);

        tcp::endpoint endpoint(asio::ip::address_v4(ip), port);
        CS_DUMP(endpoint.address().to_string());
        CS_DUMP(endpoint.port());
        us.async_connect(endpoint,
            boost::bind(&Channel::handleConnectIp, shared_from_this(),
                ADDR_IPV4, endpoint, asio::placeholders::error));
    }

    void handleDomainRequest(const boost::system::error_code &err, std::size_t bytesRead)
    {
        KICK_IF(err)

        crypto.decrypt(bufdr.data, bytesRead, bufdw.data);      // now bufdw is idle, reuse it.

        // avoid from unaligned read, and given network-bytes-order is big-endian:
        uint16_t port = (static_cast<uint16_t>(bufdw.data[bytesRead - 2]) << 8) + bufdw.data[bytesRead - 1];
        CS_DUMP(port);

        std::string domain(bufuw.data, bytesRead - 2);   // to make copyable for boost::bind.
        tcp::resolver::query query(domain, boost::lexical_cast<std::string>(port));
        resolver.async_resolve(query, boost::bind(&Channel::handleDomainResolved, shared_from_this(),
            asio::placeholders::error, asio::placeholders::iterator, domain));
    }

    void handleDomainResolved(const boost::system::error_code& err,
        tcp::resolver::iterator it, const std::string& domain)
    {
        if (CS_BLIKELY(!err))
        {
            asio::async_connect(us, it++, boost::bind(&Channel::handleConnectDomain, shared_from_this(),
                domain, it->endpoint().port(), asio::placeholders::error, it));
        }
        else
        {
            dealConnectFailed(SOCKS5_DOMAIN_RESOLVE_FAILED);
        }

    }

    void handleConnectIp(AddrType addrType, const tcp::endpoint& endpoint,
        const boost::system::error_code& err)
    {
        CS_DUMP((int)addrType);
        if (CS_BLIKELY(!err))
        {
            boost::system::error_code ec;
            tcp::endpoint usend = us.remote_endpoint(ec);
            if (CS_UNLIKELY(ec))
            {
                shutdown();
                return;
            }
            // 连接成功.
            //  +----+-----+-------+------+----------+----------+
            //  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
            //  +----+-----+-------+------+----------+----------+
            //  | 1  |  1  | X'00' |  1   | Variable |    2     |
            //  +----+-----+-------+------+----------+----------+
            //  [                                               ]
            char* dataPtr = bufdr.data; // now bufdr is idle, reuse it.
            *dataPtr++ = dsVersion;
            *dataPtr++ = CONNECT_SUCCEED;
            *dataPtr++ = 0x00;
            *dataPtr++ = addrType;
            if (CS_BLIKELY(addrType == ADDR_IPV4))
            {
                *reinterpret_cast<uint32_t*>(dataPtr + 4) = usend.address().to_v4().to_ulong();
                dataPtr += sizeof(uint32_t);
            }
            else if (addrType == ADDR_IPV6)
            {
                asio::ip::address_v6::bytes_type bytes = usend.address().to_v6().to_bytes();
                std::memcpy(dataPtr + 4, bytes.data(), bytes.size());
                dataPtr += bytes.size();
            }
            *reinterpret_cast<uint16_t*>(dataPtr) = htons(usend.port());
            int len = dataPtr - bufdr.data + sizeof(uint16_t);
            crypto.encrypt(bufdr.data, len, bufdw.data);

            asio::async_write(ds, asio::buffer(bufdw.data, len),
                asio::transfer_exactly(len),
                boost::bind(&Channel::handleConnectedResponseSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else
        {
            dealConnectFailed(err.value());
        }
    }

    void handleConnectDomain(const std::string& domain, uint16_t port,
        const boost::system::error_code &err, tcp::resolver::iterator it)
    {
        if (CS_BLIKELY(!err))
        {
            // 连接成功.
            // write
            //  +----+-----+-------+------+----------+----------+
            //  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
            //  +----+-----+-------+------+----------+----------+
            //  | 1  |  1  | X'00' |  1   | Variable |    2     |
            //  +----+-----+-------+------+----------+----------+
            //  [                                               ]
            char* dataPtr = bufdr.data;     // now bufdr is idle, lets reuse it.
            *dataPtr++ = dsVersion;
            *dataPtr++ = CONNECT_SUCCEED;
            *dataPtr++ = 0x00;
            *dataPtr++ = ADDR_DOMAIN;
            std::memcpy(dataPtr, domain.data(), domain.size());
            *reinterpret_cast<uint16_t*>(dataPtr + domain.size()) = htons(port);
            int len = 6 + domain.size();
            crypto.encrypt(bufdr.data, len, bufdw.data);

            asio::async_write(ds, asio::buffer(bufdw.data, len),
                asio::transfer_exactly(len),
                boost::bind(&Channel::handleConnectedResponseSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else
        {
            tcp::resolver::iterator end;
            if (it != end)
            {
                asio::async_connect(us, it++,
                    boost::bind(&Channel::handleConnectDomain, shared_from_this(),
                        domain, port, asio::placeholders::error, it));
                return;
            }
            else
            {
                dealConnectFailed(err);
            }
        }
    }

    void handleConnectedResponseSent(const boost::system::error_code& err, std::size_t bytesSent)
    {
        CS_SAY("told downstream connected");
        KICK_IF(err)

        ds.async_read_some(asio::buffer(bufdr.data, bufdr.capacity),
            boost::bind(&Channel::handleDsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));

        us.async_read_some(asio::buffer(bufur.data, bufur.capacity),
            boost::bind(&Channel::handleUsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleDsRead(const boost::system::error_code& err, std::size_t bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)

        crypto.decrypt(bufdr.data, bytesRead, bufuw.data);
        asio::async_write(us, asio::buffer(bufuw.data, bytesRead),
            asio::transfer_exactly(bytesRead),
            boost::bind(&Channel::handleDrSent, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUsRead(const boost::system::error_code& err, std::size_t bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)

        crypto.decrypt(bufur.data, bytesRead, bufdw.data);
        asio::async_write(ds, asio::buffer(bufdw.data, bytesRead),
            asio::transfer_exactly(bytesRead),
            boost::bind(&Channel::handleUrSent, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleDrSent(const boost::system::error_code& err, std::size_t bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)

        ds.async_read_some(asio::buffer(bufdr.data, bufdr.capacity),
            boost::bind(&Channel::handleDsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUrSent(const boost::system::error_code& err, std::size_t bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)

        us.async_read_some(asio::buffer(bufur.data, bufur.capacity),
            boost::bind(&Channel::handleUsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

private:
    void buildStage()
    {
        authority->drBufSize = authority->dwBufSize =
            authority->urBufSize = authority->uwBufSize = 32 << 10;
        bufdr.setCapacity(authority->drBufSize);
        bufdw.setCapacity(authority->dwBufSize);
        bufur.setCapacity(authority->urBufSize);
        bufuw.setCapacity(authority->uwBufSize);
    }

    uint8_t getSocksConnectErrcode(int asioErrcode) __attribute__((const))
    {
        if (CS_BLIKELY(dsVersion == PROTOCOL_V5))
        {
            switch (asioErrcode)
            {
            case asio::error::connection_refused:
            case asio::error::connection_aborted:
            case asio::error::connection_reset:
                return SOCKS5_CONNECTION_REFUSED;

            case asio::error::host_unreachable:
                return SOCKS5_HOST_UNREACHABLE;

            case asio::error::network_unreachable:
            case asio::error::network_reset:
            case asio::error::network_down:
                return SOCKS5_NETWORK_UNREACHABLE;
            }

            return SOCKS5_GENERAL_SOCKS_SERVER_FAILURE;
        }
        else
        {
            return SOCKS4_CONNECT_FAILED;
        }
    }

    void dealConnectFailed(const boost::system::error_code &err)
    {
        dealConnectFailed(getSocksConnectErrcode(err.value()));
    }

    void dealConnectFailed(const uint8_t socksErrcode)
    {
        if (CS_BLIKELY(dsVersion == PROTOCOL_V5))
        {
            // socks5 连接失败
            //  +----+-----+-------+------+----------+----------+
            //  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
            //  +----+-----+-------+------+----------+----------+
            //  | 1  |  1  | X'00' |  1   | Variable |    2     |
            //  +----+-----+-------+------+----------+----------+
            //  [                                               ]
            // just always reply with addr-type = addr-type-ipv4.
            uint8_t data[10] = { PROTOCOL_V5, socksErrcode, 0x00, ADDR_IPV4 };
            crypto.encrypt(data, sizeof(data), bufdw.data);

            asio::async_write(ds, asio::buffer(bufdw.data, sizeof(data)), asio::transfer_exactly(sizeof(data)),
                boost::bind(&Channel::shutdown, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else // if (dsVersion == PROTOCOL_V4)
        {
            // socks4 连接失败
            //  +----+----+----+----+----+----+----+----+
            //  | VN | CD | DSTPORT |      DSTIP        |
            //  +----+----+----+----+----+----+----+----+
            //  | 1  | 1  |    2    |         4         |
            //  +----+----+----+----+----+----+----+----+
            //  [                                       ]
            uint8_t data[8] = { PROTOCOL_V4, socksErrcode };
            crypto.encrypt(data, sizeof(data), bufdw.data);

            asio::async_write(ds, asio::buffer(bufdw.data, sizeof(data)), asio::transfer_exactly(sizeof(data)),
                boost::bind(&Channel::shutdown, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
    }

    void shutdown(const boost::system::error_code& err, std::size_t bytesSent)
    {
        shutdown();
    }

    void shutdown()
    {
        boost::system::error_code ignored_err;
        if (ds.is_open())
        {
            ds.shutdown(tcp::socket::shutdown_both, ignored_err);
            ds.close(ignored_err);
        }
        if (us.is_open())
        {
            us.shutdown(tcp::socket::shutdown_both, ignored_err);
            us.close(ignored_err);
        }
        if (dsu.is_open())
        {
            dsu.close();
        }
        authenticater.restore(*authority);
    }

    void addToOutlet(const std::string& username)
    {
        // TODO: lock
        if (config->multiThreads)
        {
            boost::mutex::scoped_lock lock(config->workMutex);
            _addToOutlet(username);
        }
        else
        {
            _addToOutlet(username);
        }

    }

    void _addToOutlet(const std::string& username)
    {
        UserOutletMap::iterator it = users.find(username);
        if (it == users.end())
        {
            Outlet outlet(authority);
            users.insert(std::make_pair(username, outlet));
//            users[username] = outlet;
            outlet.channels.push_back(this);
        }
        else
        {
            it->second.channels.push_back(this);
        }
    }

    bool prepare()
    {
        boost::system::error_code ec;
        {
            asio::socket_base::receive_buffer_size option(config->drBufferSize);
            ds.set_option(option, ec);
            FALSE_IF(ec);
        }
        {
            asio::socket_base::send_buffer_size option(config->dwBufferSize);
            ds.set_option(option, ec);
            FALSE_IF(ec);
        }
        {
            asio::socket_base::receive_buffer_size option(config->urBufferSize);
            us.set_option(option, ec);
            FALSE_IF(ec);
        }
        {
            asio::socket_base::send_buffer_size option(config->uwBufferSize);
            us.set_option(option, ec);
            FALSE_IF(ec);
        }
        if (config->dsTcpNodelay)
        {
            tcp::no_delay option(true);
            ds.set_option(option, ec);
            FALSE_IF(ec);
        }
        if (config->usTcpNodelay)
        {
            tcp::no_delay option(true);
            us.set_option(option, ec);
            FALSE_IF(ec);
        }
        return true;
    }

};

}

#undef KICK_IF

#undef FALSE_IF

#undef SINGLE_BYTE
