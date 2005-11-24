/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#ifndef __NETLINK_H__
#define __NETLINK_H__

#include <omnetpp.h>
#include <vector>

#include "zebra_env.h"

#include "RoutingTable.h"

// structure used by RTM_NEWROUTE and RTM_DELROUTE

struct rtm_request_t
{
    struct nlmsghdr n;
    struct rtmsg r;
    char buf[1024];
};

//

class NetlinkResult
{
    public:
        NetlinkResult() { data = 0; }

        int copyout(struct msghdr *msg);
        void copyin(char *ptr, int len);

    private:
        char *data;
        int length;
};

class Netlink
{
    public:

        // bind
        struct sockaddr_nl local;

        // fcntl
        int status;


        NetlinkResult shiftResult();
        void pushResult(const NetlinkResult& result);

        NetlinkResult listInterfaces();
        NetlinkResult listAddresses();
        NetlinkResult listRoutes(RoutingEntry *entry = NULL);
        RoutingEntry* route_command(int cmd_type, rtm_request_t* rm);

        void bind(int pid);

    private:
        std::vector<NetlinkResult> results;

        void route_del(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric);
        RoutingEntry* route_new(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric);
};

#endif

