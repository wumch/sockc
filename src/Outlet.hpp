
#pragma once

#include "predef.hpp"
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>
#include "Authority.hpp"

namespace csocks
{

class Channel;

class Outlet
{
public:
    typedef boost::ptr_vector<Channel> ChannelList;
//    typedef std::vector<Channel*> ChannelList;

    Authority* authority;
    ChannelList channels;

    explicit Outlet(Authority* _authority):
        authority(_authority)
    {}

    Outlet(const Outlet& o):
        authority(o.authority)
    {
//        channels = o.channels;
    }
};

typedef boost::unordered_map<std::string, Outlet> UserOutletMap;

}
