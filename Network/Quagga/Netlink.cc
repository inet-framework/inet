
#include "Netlink.h"
#include "Daemon.h"

#include "RoutingTable.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "oppsim_kernel.h"  // oppsim_htons() etc

extern "C" {

void netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta, int len);

};

void Netlink::bind(int pid)
{
    local.nl_pid = pid;
}


NetlinkResult Netlink::shiftResult()
{
    EV << "shifting result from queue" << endl;
    ASSERT(results.size() > 0);

    NetlinkResult r = results[0];
    results.erase(results.begin());
    return r;
}

void Netlink::pushResult(const NetlinkResult& result)
{
    EV << "adding result into queue" << endl;
    results.push_back(result);
}

NetlinkResult Netlink::listInterfaces()
{
    InterfaceTable *ift = check_and_cast<InterfaceTable*>(simulation.runningModule()->parentModule()->submodule("interfaceTable"));

    struct ret_t
    {
        struct nlmsghdr nlh;
        struct ifinfomsg ifi;
    } *ret;

    char buff[4096];

    ret = (ret_t*)buff;

    for(int i = 0; i < ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);

        const char *name = ie->name();
        int namelen = strlen(name) + 1;

        EV << "adding interface " << name << endl;

        ret->nlh.nlmsg_len = NLMSG_LENGTH(sizeof(ret->ifi));
        ret->nlh.nlmsg_type = RTM_NEWLINK;
        ret->nlh.nlmsg_flags = NLM_F_MULTI;
        ret->ifi.ifi_family = AF_INET;
        ret->ifi.__ifi_pad = 0;

        if(ie->isLoopback())
            ret->ifi.ifi_type = ARPHRD_LOOPBACK;
        else if(ie->isBroadcast())
            ret->ifi.ifi_type = ARPHRD_ETHER;
        else if(ie->isPointToPoint())
            ret->ifi.ifi_type = ARPHRD_PPP;
        else
            ASSERT(false);

        ret->ifi.ifi_index = i;
        ret->ifi.ifi_flags = 0;

        if(ie->isLoopback())
            ret->ifi.ifi_flags |= IFF_LOOPBACK;
        if(ie->isBroadcast())
            ret->ifi.ifi_flags |= IFF_BROADCAST;
        if(ie->isPointToPoint())
            ret->ifi.ifi_flags |= IFF_POINTOPOINT;
        if(ie->isMulticast())
            ret->ifi.ifi_flags |= IFF_MULTICAST;
        if(!ie->isDown())
            ret->ifi.ifi_flags |= (IFF_UP | IFF_RUNNING);

        ret->ifi.ifi_change = 0;

        ASSERT(ret->ifi.ifi_flags & IFF_UP);
        ASSERT(ret->ifi.ifi_flags & IFF_RUNNING);

        struct rtattr *rta = IFLA_RTA(NLMSG_DATA(ret));

        // add IFNAME

        rta->rta_type = IFLA_IFNAME;
        rta->rta_len = RTA_LENGTH(namelen);
        memcpy(RTA_DATA(rta), name, namelen);
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == namelen);

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        // add MTU

        rta->rta_type = IFLA_MTU;
        rta->rta_len = RTA_LENGTH(sizeof(int));
        *(int*)RTA_DATA(rta) = ie->mtu();
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == sizeof(int));

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        //

        ret = (ret_t*)(((char*)ret) + ret->nlh.nlmsg_len);

        ASSERT((char*)ret < &buff[sizeof(buff)]);
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    ret = (ret_t*)(((char*)ret) + ret->nlh.nlmsg_len);

    ASSERT((char*)ret < &buff[sizeof(buff)]);

    // calculate length

    int length = ((char*)ret) - buff;

    EV << "total message length=" << length << endl;

    // store result

    NetlinkResult ifs;
    ifs.copyin(buff, length);
    return ifs;
}

NetlinkResult Netlink::listAddresses()
{
    InterfaceTable *ift = check_and_cast<InterfaceTable*>(simulation.runningModule()->parentModule()->submodule("interfaceTable"));
    RoutingTable *rt = check_and_cast<RoutingTable*>(simulation.runningModule()->parentModule()->submodule("routingTable"));

    struct ret_t
    {
        struct nlmsghdr nlh;
        struct ifaddrmsg ifa;
    } *ret;

    char buff[4096];

    ret = (ret_t*)buff;

    for(int i = 0; i < ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);

        const char *name = ie->name();
        int namelen = strlen(name) + 1;

        EV << "adding address for interface " << name << endl;

        ret->nlh.nlmsg_len = NLMSG_LENGTH(sizeof(ret->ifa));
        ret->nlh.nlmsg_type = RTM_NEWADDR;
        ret->nlh.nlmsg_flags = NLM_F_MULTI;
        ret->ifa.ifa_family = AF_INET;
        ret->ifa.ifa_index = i;
        ret->ifa.ifa_prefixlen = ie->ipv4()->netmask().netmaskLength();
        ret->ifa.ifa_flags = 0; // only secondary used
        ret->ifa.ifa_scope = 0; // not used in Quagga at all (?)

        struct rtattr *rta = IFA_RTA(NLMSG_DATA(ret));

        // add IFA_LOCAL

        rta->rta_type = IFA_LOCAL;
        rta->rta_len = RTA_LENGTH(sizeof(uint32));
        *(uint32*)RTA_DATA(rta) = htonl(ie->ipv4()->inetAddress().getInt());
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == sizeof(uint32));

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        if(ie->isPointToPoint())
        {
            // add IFA_ADDRESS

            IPAddress peerIP;

            for(int k = 0; k < rt->numRoutingEntries(); k++)
            {
                RoutingEntry *re = rt->routingEntry(k);

                if(re->interfacePtr != ie)
                    continue;

                //if(!re->gateway.isUnspecified())
                //  continue;

                if(re->netmask.netmaskLength() != 32)
                    continue;

                if(re->type != RoutingEntry::DIRECT)
                    continue;

                peerIP = re->host;

                EV << "peer address=" << peerIP << endl;

                break;
            }

            ASSERT(!peerIP.isUnspecified());

            rta->rta_type = IFA_ADDRESS;
            rta->rta_len = RTA_LENGTH(sizeof(uint32));
            *(uint32*)RTA_DATA(rta) = htonl(peerIP.getInt());
            ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

            ASSERT(RTA_PAYLOAD(rta) == sizeof(uint32));

            rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        }

        // add IFA_BROADCAST

        if(ie->isBroadcast())
        {
            uint32 inet_addr = ie->ipv4()->inetAddress().getInt();
            uint32 mask_addr = ie->ipv4()->netmask().getInt();
            uint32 bcast_addr = inet_addr | (mask_addr ^ 0xffff);

            EV << "interfaceinfo: " << ie->info() << endl;
            EV << "detailedinterfaceinfo: " << ie->detailedInfo() << endl;

            ASSERT(inet_addr);

            rta->rta_type = IFA_ADDRESS;
            rta->rta_len = RTA_LENGTH(sizeof(uint32));
            *(uint32*)RTA_DATA(rta) = htonl(inet_addr);
            ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

            ASSERT(RTA_PAYLOAD(rta) == sizeof(uint32));

            rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

            rta->rta_type = IFA_BROADCAST;
            rta->rta_len = RTA_LENGTH(sizeof(uint32));
            *(uint32*)RTA_DATA(rta) = htonl(bcast_addr);
            ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

            ASSERT(RTA_PAYLOAD(rta) == sizeof(uint32));

            rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
        }

        //

        ret = (ret_t*)(((char*)ret) + ret->nlh.nlmsg_len);

        ASSERT((char*)ret < &buff[sizeof(buff)]);
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    ret = (ret_t*)(((char*)ret) + ret->nlh.nlmsg_len);

    ASSERT((char*)ret < &buff[sizeof(buff)]);

    // calculate length

    int length = ((char*)ret) - buff;

    EV << "total message length=" << length << endl;

    // store result

    NetlinkResult ifs;
    ifs.copyin(buff, length);
    return ifs;
}

NetlinkResult Netlink::listRoutes(RoutingEntry *entry)
{
    InterfaceTable *ift = check_and_cast<InterfaceTable*>(simulation.runningModule()->parentModule()->submodule("interfaceTable"));
    RoutingTable *rt = check_and_cast<RoutingTable*>(simulation.runningModule()->parentModule()->submodule("routingTable"));

    struct ret_t
    {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
    } *ret;

    char buff[4096];

    ret = (ret_t*)buff;

    for(int i = 0; i < rt->numRoutingEntries(); i++)
    {
        RoutingEntry *re = rt->routingEntry(i);

        if(entry && entry != re)
            continue;

        if(re->host.isMulticast())
            continue;

        EV << "listing route to " << re->host << endl;

        ret->nlh.nlmsg_len = NLMSG_LENGTH(sizeof(ret->rtm));
        ret->nlh.nlmsg_type = RTM_NEWROUTE;
        ret->nlh.nlmsg_flags = NLM_F_MULTI;
        ret->rtm.rtm_type = RTN_UNICAST;
        ret->rtm.rtm_flags = 0;

        switch(re->source)
        {
            case RoutingEntry::MANUAL:
                ret->rtm.rtm_protocol = RTPROT_STATIC;
                break;

            case RoutingEntry::IFACENETMASK:
                ret->rtm.rtm_protocol = RTPROT_BOOT;
                break;

            case RoutingEntry::ZEBRA:
                ret->rtm.rtm_protocol = RTPROT_ZEBRA;
                break;

            default:
                ASSERT(false);
        }

        ret->rtm.rtm_src_len = 0; // dunno why, see rt_netlink.c
        ret->rtm.rtm_family = AF_INET;

        struct rtattr *rta = RTM_RTA(NLMSG_DATA(ret));

        // add RTA_OIF (?)

        rta->rta_type = RTA_OIF;
        rta->rta_len = RTA_LENGTH(sizeof(int));
        *(int*)RTA_DATA(rta) = re->interfacePtr->interfaceId();
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == sizeof(int));

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        // add RTA_DST

        const char *dststr = re->host.str().data();
        int len1 = strlen(dststr) + 1;

        rta->rta_type = RTA_DST;
        rta->rta_len = RTA_LENGTH(len1);
        memcpy(RTA_DATA(rta), dststr, len1);
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == len1);

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));

        // add RTA_PRIORITY

        rta->rta_type = RTA_PRIORITY;
        rta->rta_len = RTA_LENGTH(sizeof(int));
        *(int*)RTA_DATA(rta) = re->metric;
        ret->nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

        ASSERT(RTA_PAYLOAD(rta) == sizeof(int));

        rta = (rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    ret = (ret_t*)(((char*)ret) + ret->nlh.nlmsg_len);

    ASSERT((char*)ret < &buff[sizeof(buff)]);

    // calculate length

    int length = ((char*)ret) - buff;

    EV << "total message length=" << length << endl;

    // store result

    NetlinkResult ifs;
    ifs.copyin(buff, length);
    return ifs;
}

int NetlinkResult::copyout(struct msghdr *message)
{
    ASSERT(message);
    ASSERT(data);

    ASSERT(message->msg_iovlen > 0);
    ASSERT(message->msg_iov[0].iov_len >= length);

    //
    struct sockaddr_nl *nl = (sockaddr_nl*)message->msg_name;
    nl->nl_pid = 0;
    nl->nl_family = AF_NETLINK;
    message->msg_namelen = sizeof *nl;

    //
    memcpy(message->msg_iov[0].iov_base, data, length);

    //
    message->msg_flags = 0;

    free(data);
    data = 0;

    return length;
}

void NetlinkResult::copyin(char *ptr, int len)
{
    ASSERT(ptr);
    ASSERT(!data);

    data = (char*)malloc(len);
    memcpy(data, ptr, len);
    length = len;
}


RoutingEntry* Netlink::route_command(int cmd_type, rtm_request_t* rm)
{
    ASSERT(rm->n.nlmsg_type == RTM_NEWROUTE || rm->n.nlmsg_type == RTM_DELROUTE);

    // code based on rt_netlink.c

    // parse command

    struct rtattr *tb[RTA_MAX + 1];

    ASSERT(rm->n.nlmsg_len >= NLMSG_LENGTH (sizeof (struct rtmsg)));
    ASSERT(rm->n.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK));
    ASSERT(rm->r.rtm_family == AF_INET);
    //rm->r.rtm_table =
    //rm->r.rtm_dst_len =
    ASSERT(rm->r.rtm_scope == RT_SCOPE_UNIVERSE);

    if(rm->n.nlmsg_type == RTM_NEWROUTE)
    {
        ASSERT(rm->r.rtm_protocol == RTPROT_ZEBRA);
        ASSERT(rm->r.rtm_type == RTN_UNICAST);
    }
    else
    {
        ASSERT(rm->r.rtm_protocol == RTPROT_UNSPEC);
        ASSERT(rm->r.rtm_type == RTN_UNSPEC);
    }

    ssize_t len = rm->n.nlmsg_len - NLMSG_LENGTH (sizeof (struct rtmsg));
    ASSERT(len >= 0);

    memset (tb, 0, sizeof tb);
    netlink_parse_rtattr (tb, RTA_MAX, RTM_RTA (&rm->r), len);

    IPAddress destAddr;
    IPAddress gwAddr;
    IPAddress netmaskAddr;
    int metric = -1;
    int index = -1;

    if(tb[RTA_DST])
    {
        void *dest = RTA_DATA(tb[RTA_DST]);
        uint32 d;
        memcpy(&d, dest, sizeof(d));
        destAddr = IPAddress(htonl(d));
    }

    if(tb[RTA_GATEWAY])
    {
        void *gateway = RTA_DATA(tb[RTA_GATEWAY]);
        uint32 g;
        memcpy(&g, gateway, sizeof(g));
        gwAddr = IPAddress(htonl(g));
    }

    if(tb[RTA_PRIORITY])
    {
        metric = *(int *) RTA_DATA(tb[RTA_PRIORITY]);
    }

    if (tb[RTA_OIF])
    {
        index = *(int *) RTA_DATA (tb[RTA_OIF]);
    }

    netmaskAddr = IPAddress(IPAddress("255.255.255.255").getInt() ^ ((1 << (32 - rm->r.rtm_dst_len)) - 1));

    // execute command

    switch(rm->n.nlmsg_type)
    {
        case RTM_NEWROUTE:
            return route_new(destAddr, netmaskAddr, gwAddr, index, metric);

        case RTM_DELROUTE:
            route_del(destAddr, netmaskAddr, gwAddr, index, metric);
            return NULL;
    }

    ASSERT(false);
}

void Netlink::route_del(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric)
{
    Daemon *libm = DAEMON;

    RoutingTable *rt = check_and_cast<RoutingTable*>(libm->parentModule()->submodule("routingTable"));
    InterfaceTable *ift = check_and_cast<InterfaceTable*>(libm->parentModule()->submodule("interfaceTable"));

    RoutingEntry *re = rt->findRoutingEntry(destAddr, netmaskAddr, gwAddr, metric);
    ASSERT(re);
    ASSERT(index < 0 || rt->routingEntry(index) == re);

    EV << "removing routing entry:" << re->info() << endl;

    bool done = rt->deleteRoutingEntry(re);
    ASSERT(done);
}

RoutingEntry* Netlink::route_new(IPAddress destAddr, IPAddress netmaskAddr, IPAddress gwAddr, int index, int metric)
{
    Daemon *libm = DAEMON;

    RoutingTable *rt = check_and_cast<RoutingTable*>(libm->parentModule()->submodule("routingTable"));
    InterfaceTable *ift = check_and_cast<InterfaceTable*>(libm->parentModule()->submodule("interfaceTable"));

    RoutingEntry *re = new RoutingEntry();
    re->host = destAddr;
    re->netmask = netmaskAddr;
    re->gateway = gwAddr;

    if(index < 0)
    {
        ASSERT(!re->gateway.isUnspecified());
        ASSERT(rt->interfaceForDestAddr(re->gateway)!=NULL);
        re->interfacePtr = rt->interfaceForDestAddr(re->gateway);
    }
    else
        re->interfacePtr = ift->interfaceAt(index);

    re->interfaceName = re->interfacePtr->name();

    if(re->gateway.isUnspecified())
        re->type = RoutingEntry::DIRECT;
    else
        re->type = RoutingEntry::REMOTE;

    re->source = RoutingEntry::ZEBRA;

    re->metric = metric;

    EV << "adding routing entry:" << re->info() << endl;

    rt->addRoutingEntry(re);

    return re;
}

