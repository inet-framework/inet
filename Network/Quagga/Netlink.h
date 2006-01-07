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

#include "InterfaceTable.h"
#include "RoutingTable.h"

// netlink attribute manipulation ********************************************

struct ret_t
{
    struct nlmsghdr nlh;
    union
    {
        struct ifaddrmsg ifa;
        struct ifinfomsg ifi;
        struct rtmsg rtm;
    };
    char buf[4096];
};

extern "C" {

    // borrowed from quaggasrc
    void netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta, int len);

};

// NetlinkResult class *******************************************************


class NetlinkResult
{
    public:
        NetlinkResult() { data = 0; }
        NetlinkResult(void *ptr, int len) { data = 0; copyin(ptr, len); }

        int copyout(struct msghdr *msg);
        void copyin(void *ptr, int len);

    private:
        char *data;
        int length;
};

// Netlink class *************************************************************

class Netlink
{
    public:

        Netlink();

        // bind
        struct sockaddr_nl local;

        // fcntl
        int status;


        NetlinkResult getNextResult();
        void appendResult(const NetlinkResult& result);

        NetlinkResult listInterfaces();
        NetlinkResult listAddresses();
        NetlinkResult listRoutes(RoutingEntry *entry = NULL);
        RoutingEntry* route_command(int cmd_type, ret_t* rm);

        void bind(int pid);

    private:
        std::list<NetlinkResult> results;

        InterfaceTable *ift;
        RoutingTable *rt;

        void route_del(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric);
        RoutingEntry* route_new(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric);
};

#endif

