
#pragma once

#include "predef.hpp"
#if _CSOCKS_IS_LINUX
extern "C" {
#   include <unistd.h>
}
#endif
#include <exception>
#include <fstream>
#include <boost/filesystem.hpp>
#include "Config.hpp"
#include "Bus.hpp"

using boost::asio::ip::tcp;

namespace csocks
{

class Portal
{
private:
    Bus bus;
    const Config* const config;
#if _CSOCKS_IS_LINUX
    bool pidFileSelfCreated;
#endif

public:
    Portal():
        config(Config::instance()), pidFileSelfCreated(false)
    {}

    void run()
    {
#if _CSOCKS_IS_LINUX
        savePid();
#endif
        bus.start();
    }

#if _CSOCKS_IS_LINUX
    ~Portal()
    {
        rmPidFile();
    }
#endif

#if _CSOCKS_IS_LINUX
private:
    void savePid()
    {
        if (boost::filesystem::exists(config->pidFile))
        {
            if (boost::filesystem::file_size(config->pidFile) > 0)
            {
                CS_DIE("pid-file [" << config->pidFile << "] already exists and is not empty");
            }
        }

        boost::filesystem::path dir(config->pidFile.parent_path());
        if (!boost::filesystem::is_directory(dir))
        {
            try
            {
                boost::filesystem::create_directories(dir);
            }
            catch (const std::exception& e)
            {
                CS_DIE(e.what());
            }
        }

        try
        {
            pidFileSelfCreated = boost::filesystem::exists(config->pidFile);
            std::ofstream of(config->pidFile.c_str());
            if (!of.is_open())
            {
                CS_DIE("failed on opening pid-file [" << config->pidFile << "]");
            }
            of << getpid() << std::endl;
            of.close();
            if (!of.good())
            {
                CS_DIE("failed on writing pid to [" << config->pidFile << "]");
            }
            CS_SAY("pid written to [" << config->pidFile << "]");
        }
        catch (const std::exception& e)
        {
            CS_DIE(e.what());
        }
    }

    void rmPidFile()
    {
        if (pidFileSelfCreated)
        {
            if (boost::filesystem::exists(config->pidFile))
            {
                boost::filesystem::remove(config->pidFile);
            }
        }
    }
#endif
};

}
