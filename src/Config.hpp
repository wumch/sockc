
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>

extern int main(int, char*[]);

namespace csocks
{

class Config
{
    friend int ::main(int, char*[]);
private:
    static Config _instance;

    Config():
        desc("allowed config options")
    {}

    void load(boost::filesystem::path file);

    static Config* mutableInstance()
    {
        return &_instance;
    }

    void initDesc();

public:
    static const Config* instance()
    {
        return &_instance;
    }

    void init(int argc, char* argv[]);

public:
    std::string programName;
    std::size_t workerCount;
    std::size_t ioThreads;
#ifdef __linux__
    boost::filesystem::path pidFile;
    std::size_t stackSize;
#endif

    bool reuseAddress;
    std::size_t maxConnections;
    std::size_t backlog;
    bool dsTcpNodelay, usTcpNodelay;

    std::size_t ioServiceNum;
    boost::asio::ip::address host;
    uint16_t port;

    boost::asio::ip::address usHost;
    uint16_t usPort;

    std::string username, password;

    std::time_t dsRecvTimeout, dsSendTimeout,
        usRecvTimeout, usSendTimeout;

    std::size_t drBufferSize, dwBufferSize,
        urBufferSize, uwBufferSize;

    bool multiThreads, multiIoThreads;
    mutable boost::mutex workMutex, ioMutex;


    boost::program_options::variables_map options;
    boost::program_options::options_description desc;
};

}