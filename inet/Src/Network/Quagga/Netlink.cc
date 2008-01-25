
#include "Netlink.h"
#include "Daemon.h"
#include "oppsim_kernel.h"

#include "IPv4InterfaceData.h"

#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"

// Netlink result ************************************************************

int NetlinkResult::copyout(struct msghdr *message)
{
    ASSERT(message);
    ASSERT(data);

    ASSERT(message->msg_iovlen > 0);
    ASSERT(message->msg_iov[0].iov_len >= length);

    // copy stored result into output buffer

    memcpy(message->msg_iov[0].iov_base, data, length);

    // set source address, flags etc.

    struct sockaddr_nl *nl = (sockaddr_nl*)message->msg_name;
    nl->nl_pid = 0;
    nl->nl_family = AF_NETLINK;
    message->msg_namelen = sizeof(struct sockaddr_nl);
    message->msg_flags = 0;

    // free stored result

    free(data);
    data = 0;

    return length;
}

void NetlinkResult::copyin(void *ptr, int len)
{
    ASSERT(ptr);
    ASSERT(!data);

    data = (char*)malloc(len);
    memcpy(data, ptr, len);
    length = len;
}

// netlink attribute helpers *************************************************

static void rta_add_param(rtattr** rta, unsigned short type, unsigned short len, const void *ptr, ret_t* ret)
{
    (*rta)->rta_type = type;
    (*rta)->rta_len = RTA_LENGTH(len);
    memcpy(RTA_DATA(*rta), ptr, len);
    ret->nlh.nlmsg_len += RTA_ALIGN((*rta)->rta_len);
    *rta = (rtattr*)(((char*)*rta) + RTA_ALIGN((*rta)->rta_len));
}

static void rta_end_param(ret_t **ret)
{
    *ret = (ret_t*)(((char*)*ret) + (*ret)->nlh.nlmsg_len);
}

// ***************************************************************************

Netlink::Netlink()
{
    RoutingTableAccess rtAccess;
    rt = rtAccess.get();

    InterfaceTableAccess iftAccess;
    ift = iftAccess.get();
}

void Netlink::bind(int pid)
{
    local.nl_pid = pid;
}

NetlinkResult Netlink::getNextResult()
{
    ASSERT(!results.empty());
    NetlinkResult r = *results.begin();
    results.pop_front();
    return r;
}

void Netlink::appendResult(const NetlinkResult& result)
{
    results.push_back(result);
}

NetlinkResult Netlink::listInterfaces()
{
    struct ret_t retvar;
    struct ret_t *ret = &retvar;

    for(int i = 0; i < ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);

        const char *name = ie->name();
        int mtu = ie->mtu();

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

        rta_add_param(&rta, IFLA_IFNAME, strlen(name) + 1, name, ret);
        rta_add_param(&rta, IFLA_MTU, sizeof(mtu), &mtu, ret);

        rta_end_param(&ret);
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    rta_end_param(&ret);

    // calculate length

    int length = ((char*)ret) - (char*)&retvar;

    EV << "total message length=" << length << endl;

    ASSERT(length < sizeof(retvar));

    // store result

    NetlinkResult ifs(&retvar, length);

    return ifs;
}

//

static IPAddress findPeerInRoutingTable(InterfaceEntry *localInf, RoutingTable *rt)
{
    ASSERT(localInf);
    ASSERT(localInf->isPointToPoint());
    ASSERT(rt);

    for(int k = 0; k < rt->numRoutingEntries(); k++)
    {
        RoutingEntry *re = rt->routingEntry(k);

        if(re->interfacePtr != localInf)
            continue;

        //if(!re->gateway.isUnspecified())
        //  continue;

        if(re->netmask.netmaskLength() != 32)
            continue;

        if(re->type != RoutingEntry::DIRECT)
            continue;

        EV << "peer address=" << re->host << endl;

        return re->host;
    }

    return IPAddress();
}

// ******************

NetlinkResult Netlink::listAddresses()
{
    struct ret_t retvar;
    struct ret_t *ret =  &retvar;

    for(int i = 0; i < ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);

        const char *name = ie->name();

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

        uint32 local_addr = htonl(ie->ipv4()->inetAddress().getInt());
        rta_add_param(&rta, IFA_LOCAL, sizeof(local_addr), &local_addr, ret);

        if(ie->isPointToPoint())
        {
            // XXX this is temporary solution, until proper PPP layer support is added
            // you have to add point-to-point links into routing table manually
            IPAddress peerIP = findPeerInRoutingTable(ie, rt);
            ASSERT(!peerIP.isUnspecified());

            if(!peerIP.isUnspecified())
            {
                uint32 addr = htonl(peerIP.getInt());
                rta_add_param(&rta, IFA_ADDRESS, sizeof(addr), &addr, ret);
            }
        }

        if(ie->isBroadcast())
        {
            uint32 inet_addr = ie->ipv4()->inetAddress().getInt();
            uint32 mask_addr = ie->ipv4()->netmask().getInt();
            uint32 bcast_addr = inet_addr | (mask_addr ^ 0xffff);

            inet_addr = htonl(inet_addr);
            bcast_addr = htonl(bcast_addr);

            rta_add_param(&rta, IFA_ADDRESS, sizeof(inet_addr), &inet_addr, ret);
            rta_add_param(&rta, IFA_BROADCAST, sizeof(bcast_addr), &bcast_addr, ret);
        }

        rta_end_param(&ret);
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    rta_end_param(&ret);

    // calculate length

    int length = ((char*)ret) - (char*)&retvar;

    EV << "total message length=" << length << endl;

    ASSERT(length < sizeof(retvar));

    // store result

    NetlinkResult ifs(&retvar, length);

    return ifs;
}

NetlinkResult Netlink::listRoutes(RoutingEntry *entry)
{
    struct ret_t retvar;
    struct ret_t *ret = &retvar;

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

        int interfaceId = re->interfacePtr->interfaceId();
        int metric = re->metric;
        const char *dststr = re->host.str().data();

        struct rtattr *rta = RTM_RTA(NLMSG_DATA(ret));

        rta_add_param(&rta, RTA_OIF, sizeof(interfaceId), &interfaceId, ret);
        rta_add_param(&rta, RTA_DST, strlen(dststr) + 1, dststr, ret);
        rta_add_param(&rta, RTA_PRIORITY, sizeof(metric), &metric, ret);

        rta_end_param(&ret);
    }

    // add footer

    ret->nlh.nlmsg_len = NLMSG_LENGTH(0);
    ret->nlh.nlmsg_type = NLMSG_DONE;
    ret->nlh.nlmsg_flags = 0;

    rta_end_param(&ret);

    // calculate length

    int length = ((char*)ret) - (char*)&retvar;

    EV << "total message length=" << length << endl;

    ASSERT(length < sizeof(retvar));

    // store result

    NetlinkResult ifs(&retvar, length);

    return ifs;
}



RoutingEntry* Netlink::route_command(int cmd_type, ret_t* rm)
{
    ASSERT(rm->nlh.nlmsg_type == RTM_NEWROUTE || rm->nlh.nlmsg_type == RTM_DELROUTE);

    // code based on rt_netlink.c

    // parse command

    struct rtattr *tb[RTA_MAX + 1];

    ASSERT(rm->nlh.nlmsg_len >= NLMSG_LENGTH (sizeof (struct rtmsg)));
    ASSERT(rm->nlh.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK));
    ASSERT(rm->rtm.rtm_family == AF_INET);
    //rm->r.rtm_table =
    //rm->r.rtm_dst_len =
    ASSERT(rm->rtm.rtm_scope == RT_SCOPE_UNIVERSE);

    if(rm->nlh.nlmsg_type == RTM_NEWROUTE)
    {
        ASSERT(rm->rtm.rtm_protocol == RTPROT_ZEBRA);
        ASSERT(rm->rtm.rtm_type == RTN_UNICAST);
    }
    else
    {
        ASSERT(rm->rtm.rtm_protocol == RTPROT_UNSPEC);
        ASSERT(rm->rtm.rtm_type == RTN_UNSPEC);
    }

    ssize_t len = rm->nlh.nlmsg_len - NLMSG_LENGTH (sizeof (struct rtmsg));
    ASSERT(len >= 0);

    memset (tb, 0, sizeof tb);
    netlink_parse_rtattr (tb, RTA_MAX, RTM_RTA (&rm->rtm), len);

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

    netmaskAddr = IPAddress(IPAddress("255.255.255.255").getInt() ^ ((1 << (32 - rm->rtm.rtm_dst_len)) - 1));

    // execute command

    switch(rm->nlh.nlmsg_type)
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

