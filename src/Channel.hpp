
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
#include <cryptopp/osrng.h>
#include "stage/backtrace.hpp"
#include "Authenticater.hpp"
#include "Config.hpp"
#include "Crypto.hpp"
#include "Buffer.hpp"
#include "Outlet.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

#define SINGLE_BYTE __attribute__((aligned(1), packed))

#define KICK_IF(err) if (CS_UNLIKELY(err)) { CS_SAY("[" << (uint64_t)this << "] will return"); return; }

#define FALSE_IF(err) if (CS_UNLIKELY(ec)) { CS_SAY("[" << (uint64_t)this << "] will return false."); return false; }

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
        KICK_IF(!prepareDs());
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
        CS_DUMP(bufdr.capacity);
        asio::async_read(ds, asio::buffer(bufdr.data, 2),
            asio::transfer_exactly(2),
            boost::bind(&Channel::handleDsNumMethodsRead, shared_from_this(), asio::placeholders::error,
                asio::placeholders::bytes_transferred));
    }

    void handleDsNumMethodsRead(const boost::system::error_code& err, int bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)
        KICK_IF(bytesRead != 2);

        const char* const header = bufdr.data;
        dsVersion = header[0];
        CS_DUMP((int)dsVersion);
        CS_DUMP((int)header[1]);
        KICK_IF(dsVersion != PROTOCOL_V5 && dsVersion != PROTOCOL_V4);
        KICK_IF(maxNumMethods < header[1]);

        if (header[1])
        {
            // read
            //  +----+----------+----------+
            //  |VER | NMETHODS | METHODS  |
            //  +----+----------+----------+
            //  | 1  |    1     | 1 to 255 |
            //  +----+----------+----------+
            //                  [          ]
            asio::async_read(ds, asio::buffer(bufdr.data, header[1]), asio::transfer_exactly(header[1]),
                boost::bind(&Channel::handleMethods, shared_from_this(), header[1],
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        else
        {
            connectUs();
        }
    }

    // received [num-methods,methods] from downstream
    void handleMethods(int numMethods, const boost::system::error_code& err, int bytesRead)
    {
        CS_DUMP(numMethods);
        KICK_IF(err)
        KICK_IF(bytesRead != numMethods)

        const char* const methods = bufdr.data;

        uint8_t method;
        if (CS_BLIKELY(methods[0] == AUTH_NONE))
        {
            method = AUTH_NONE;
        }
        else
        {
            method = AUTH_UNACCEPTABLE;
            for (int i = 0; i < numMethods; ++i)
            {
                if (methods[i] == AUTH_NONE)
                {
                    method = methods[i];
                    break;
                }
            }
        }
        CS_DUMP((int)method);

        if (method == AUTH_NONE)
        {
            connectUs();
        }
    }

    // after received [num-methods,methods] from downstream, connecting to upstream.
    void connectUs()
    {
        CS_SAY("connecting to upstream");
        // connect to upstream
        us.async_connect(tcp::endpoint(config->usHost, config->usPort),
            boost::bind(&Channel::handleUsConnected, shared_from_this(), asio::placeholders::error));
    }

    // after upstream connected
    void handleUsConnected(const boost::system::error_code& err)
    {
        CS_SAY("upstream connected");
        KICK_IF(err);
        KICK_IF(!prepareUs());

        // write to upstream:
        //  +----+----------+----------+----------+----------+
        //  |VER | NMETHODS |   KEY    |    IV    | METHODS  |
        //  +----+----------+----------+----------+----------+
        //  | 1  |    1     |   16     |    16    | 1 to 255 |
        //  +----+----------+----------+----------+----------+
        //                                        [          ]
        uint8_t data[2] = {dsVersion, 1};

        uint8_t key[CryptoPP::AES::DEFAULT_KEYLENGTH];
        uint8_t iv[CryptoPP::AES::BLOCKSIZE];
        {
            CryptoPP::AutoSeededRandomPool rnd;
            rnd.GenerateBlock(key, sizeof(key));
            rnd.GenerateBlock(iv, sizeof(iv));
            crypto.setDecKeyWithIv(key, sizeof(key), iv, sizeof(iv));
            crypto.setEncKeyWithIv(key, sizeof(key), iv, sizeof(iv));
            crypto.encrypt(data, sizeof(data), bufuw.data);
        }
        std::memcpy(bufuw.data + sizeof(data), key, sizeof(key));
        std::memcpy(bufuw.data + (sizeof(data) + sizeof(key)), iv, sizeof(iv));
        uint8_t methods[1] = {AUTH_USERPASS};
        crypto.encrypt(methods, sizeof(methods), bufuw.data + (sizeof(data) + sizeof(key) + sizeof(iv)));

        const int len = sizeof(data) + sizeof(key) + sizeof(iv) + sizeof(methods);
        CS_DUMP(len);
        asio::async_write(us, asio::buffer(bufuw.data, len), asio::transfer_exactly(len),
            boost::bind(&Channel::handleMethodSent, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    // after sent [acceptable authentication methods] to upstream.
    void handleMethodSent(const boost::system::error_code& err, int bytesSent)
    {
        CS_SAY("method sent");
        KICK_IF(err);

        asio::async_read(us, asio::buffer(bufur.data, 2), asio::transfer_exactly(2),
            boost::bind(&Channel::handleMethodDesignated, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    // received [authentication method] from upstream.
    void handleMethodDesignated(const boost::system::error_code& err, int bytesRead)
    {
        CS_SAY("received designated method from upstream");
        KICK_IF(err);

        char data[2];
        crypto.decrypt(bufur.data, 2, data);
        CS_DUMP((int)data[1]);
        if (CS_BLIKELY(data[1] == AUTH_USERPASS))
        {
            // write
            //  +----+------+----------+------+----------+
            //  |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
            //  +----+------+----------+------+----------+
            //  | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
            //  +----+------+----------+------+----------+
            //  [                                        ]
            char* ptr = bufur.data;
            *ptr++ = dsVersion;
            *ptr++ = config->username.size();
            std::memcpy(ptr, config->username.data(), config->username.size());
            ptr += config->username.size();
            *ptr++ = config->password.size();
            std::memcpy(ptr, config->password.data(), config->password.size());
            const int len = ptr - bufur.data + config->password.size();
            crypto.encrypt(bufur.data, len, bufuw.data);
            asio::async_write(us, asio::buffer(bufuw.data, len), asio::transfer_exactly(len),
                boost::bind(&Channel::handleUserPassSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        // TODO: work arround for authentication not required.
        // otherwise shutdown automatically.
    }

    void handleUserPassSent(const boost::system::error_code& err, int bytesSent)
    {
        CS_SAY("username/password sent");
        KICK_IF(err);
        asio::async_read(us, asio::buffer(bufur.data, 2), asio::transfer_exactly(2),
            boost::bind(&Channel::handleAuthed, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleAuthed(const boost::system::error_code& err, int bytesRead)
    {
        CS_SAY("authed");
        KICK_IF(err);

        char data[2];
        crypto.decrypt(bufur.data, 2, data);
        CS_DUMP((int)data[1]);
        if (CS_BLIKELY(data[1] == AUTH_RES_SUCCESS))
        {
            // write to downstream with {methods:0}:
            //  +----+----------+
            //  |VER |  METHOD  |
            //  +----+----------+
            //  | 1  |    1     |
            //  +----+----------+
            //  [               ]
            bufdw.data[0] = dsVersion;
            bufdw.data[1] = AUTH_NONE;
            asio::async_write(ds, asio::buffer(bufdw.data, 2), asio::transfer_exactly(2),
                boost::bind(&Channel::handleAuthedSent, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        // otherwise shutdown automatically.
    }

    void handleAuthedSent(const boost::system::error_code& err, std::size_t bytesSent)
    {
        CS_SAY("pingpong");
        KICK_IF(err);

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
        KICK_IF(err);

        crypto.encrypt(bufdr.data, bytesRead, bufuw.data);
        asio::async_write(us, asio::buffer(bufuw.data, bytesRead),
            asio::transfer_exactly(bytesRead),
            boost::bind(&Channel::handleDsWritten, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUsRead(const boost::system::error_code& err, std::size_t bytesRead)
    {
        CS_DUMP(bytesRead);
        KICK_IF(err)

        crypto.decrypt(bufur.data, bytesRead, bufdw.data);
        asio::async_write(ds, asio::buffer(bufdw.data, bytesRead),
            asio::transfer_exactly(bytesRead),
            boost::bind(&Channel::handleUsWritten, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleDsWritten(const boost::system::error_code& err, std::size_t bytesSent)
    {
        CS_DUMP(bytesSent);
        KICK_IF(err)

        ds.async_read_some(asio::buffer(bufdr.data, bufdr.capacity),
            boost::bind(&Channel::handleDsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

    void handleUsWritten(const boost::system::error_code& err, std::size_t bytesSent)
    {
        CS_DUMP(bytesSent);
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
            // socks4 connect failed
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

    bool prepareDs()
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
        if (config->dsTcpNodelay)
        {
            tcp::no_delay option(true);
            ds.set_option(option, ec);
            FALSE_IF(ec);
        }
        return true;
    }

    bool prepareUs()
    {
        boost::system::error_code ec;
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
