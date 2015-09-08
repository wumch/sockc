
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "Config.hpp"
#include "Channel.hpp"

namespace csocks
{

class Bus:
    public boost::noncopyable
{
private:
    const Config* const config;
    boost::asio::io_service ioService;
    tcp::acceptor acceptor;

public:
    Bus():
        config(Config::instance()),
        ioService(config->ioServiceNum),
        acceptor(ioService, tcp::endpoint(config->host, config->port))
    {}

    void start()
    {
        startAccept();
        ioService.run();
    }

private:
    void startAccept()
    {
        boost::shared_ptr<Channel> channel(new Channel(ioService));
        acceptor.async_accept(channel->downstream(),
            boost::bind(&Bus::handleAccept, this, asio::placeholders::error, channel));
    }

    void handleAccept(const boost::system::error_code& err, boost::shared_ptr<Channel>& channel)
    {
        CS_SAY("accepted");
        if (CS_BLIKELY(!err))
        {
            channel->start();
        }
        startAccept();
    }
};

}
