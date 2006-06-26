
#include <omnetpp.h>

#include "zebra_env.h"

#include "oppsim_kernel.h"

#include "Daemon.h"

#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"

#include "SocketMsg.h"

#include "UDPControlInfo_m.h"
#include "TCPCommand_m.h"
#include "IPControlInfo.h"

#include <algorithm>

// ***************

time_t zero_time = 0; // start of simulation run or whatever

Daemon *current_module = NULL;
struct GlobalVars *__activeVars = NULL;

// ****************

#ifdef _MSC_VER

//XXX #define snprintf _snprintf ?
int snprintf (char *s, size_t maxlen, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(s, maxlen, format, args);
    va_end(args);
    return n;
}

//XXX #define vsnprintf _vsnprintf ?
int vsnprintf(char *s, size_t maxlen, const char *format, va_list arg)
{
    return _vsnprintf(s, maxlen, format, arg);
}

//XXX #define strncasecmp _strnicmp ?
int strncasecmp(const char *s1, const char *s2, size_t n)
{
    return _strnicmp(s1, s2, n);
}

#endif

struct cmsghdr * __cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{
    if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
        return 0;

    __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg + CMSG_ALIGN (__cmsg->cmsg_len));
    if ((unsigned char *) (__cmsg + 1) > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)
        || ((unsigned char *) __cmsg + CMSG_ALIGN (__cmsg->cmsg_len) > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))
        return 0;
        return __cmsg;
}

static std::vector<struct servent> * services = NULL;

static void add_service(char *name, int port, char *proto)
{
	struct servent ser;
	ser.s_name = name;
	ser.s_aliases = NULL;
	ser.s_port = htons(port);
	ser.s_proto = proto;
	(*services).push_back(ser);
}

static void initialize_services() 
{
	if(services) return;
	
	// FIXME load settings from /etc/services
	// (also services must be per-daemon then)
	
	services = new std::vector<struct servent>;
	
	add_service("zebrasrv", 2600, "tcp");
	add_service("zebra", 2601, "tcp");
	add_service("ripd", 2602, "tcp");
	add_service("ospfd", 2604, "tcp");
	add_service("router", 520, "udp");	
	add_service("ospfapi", 2607, "tcp");    
};

struct servent *oppsim_getservbyname(const char *name, const char *proto)
{
    initialize_services();
	
    for(int i = 0; (*services).size(); i++)
    {
        if(strcmp(name, (*services)[i].s_name))
            continue;

        if(strcmp(proto, (*services)[i].s_proto))
            continue;

        return &(*services)[i];
    }

    ASSERT(false);

    return NULL;
}


long int oppsim_random(void)
{
    return intrand(ULONG_MAX);
}

time_t oppsim_time(time_t *tloc)
{
    time_t val = zero_time + (int)simulation.simTime();
    if(tloc)
    {
        *tloc = val;
    }
    return val;
}

char *oppsim_crypt(const char *key, const char *salt)
{
    // XXX FIXME crypt usage should be avoided by means of quagga configuration
    ASSERT(false);
    return NULL;
}

int oppsim_setregid(gid_t rgid, gid_t egid)
{
    return 0;
}

int oppsim_setreuid(uid_t ruid, uid_t euid)
{
    return 0;
}

int oppsim_seteuid(uid_t uid)
{
    return 0;
}

uid_t oppsim_getuid(void)
{
    ASSERT(false);
    return 0;
}

uid_t oppsim_geteuid(void)
{
    int euid = DAEMON->euid;

    EV << "geteuid: return " << euid << endl;

    return euid;
}

mode_t oppsim_umask(mode_t cmask)
{
    return 0755;
}

void oppsim_exit(int status)
{
    // XXX FIXME delete current module
    ASSERT(false);
}

pid_t oppsim_getpid(void)
{
    return DAEMON->id();
}

int oppsim_daemon (int nochdir, int noclose)
{
    return 0;
}

void oppsim_srand(unsigned seed)
{
}

int oppsim_gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    ASSERT(tp);
    ASSERT(!tzp);

    double ctime = simulation.simTime();

    tp->tv_sec = zero_time + (int)ctime;
    tp->tv_usec = floor((ctime - floor(ctime)) * 1000000);

    return 0;
}

void oppsim_sync(void)
{
}

int oppsim_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "setsockopt: socket=" << socket << " level=" << level << " option_name=" << option_name <<
            " option_value=" << option_value << " option_len=" << option_len << endl;

    // XXX FIXME IGNORE(OPTION) MACRO

    if(level == SOL_SOCKET && option_name == SO_RCVBUF)
    {
        EV << "SO_RCVBUF option, ignore" << endl;
        return 0;
    }

    if(level == SOL_SOCKET && option_name == SO_REUSEADDR)
    {
        EV << "SO_REUSEADDR option, ignore" << endl;
        return 0;
    }

    if(level == SOL_SOCKET && option_name == SO_BROADCAST)
    {
        EV << "SO_BROADCAST option, ignore" << endl;
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_IF)
    {
        // if this assert fails, make sure Zebra is not compiled with struct ip_mreqn here
        ASSERT(option_len == sizeof(struct in_addr));

        const struct in_addr *iaddr  = (in_addr*)option_value;

        IPAddress addr = IPAddress(ntohl(iaddr->s_addr));
        EV << "IP_MULTICAST_IF: " << addr << endl;

        RoutingTable *rt = RoutingTableAccess().get();

        InterfaceEntry *ie = rt->interfaceByAddress(addr);
        if (!ie)
        {
            // don't panic, for point-to-point links destination address is used

            for(int i = 0; i < rt->numRoutingEntries(); i++)
            {
                RoutingEntry *re = rt->routingEntry(i);

                if(!re->host.equals(addr))
                    continue;

                if(!re->interfacePtr->isPointToPoint())
                    continue;

                // interface found

                ie = re->interfacePtr;
                break;
            }
        }
        if (!ie)
        {
            // now it's offical, interface not found

            opp_error("there is no interface with address %s", addr.str().c_str());
        }

        UDPSocket *udp = dm->getIfUdpSocket(socket);
        if(udp)
        {
            udp->setMulticastInterface(ie->interfaceId());

            return 0;
        }

        RawSocket *raw = dm->getIfRawSocket(socket);
        if(raw)
        {
            raw->setMulticastInterface(ie->interfaceId());

            return 0;
        }

        ASSERT(false);
    }

    if(level == 0 && option_name == IP_ADD_MEMBERSHIP)
    {
        EV << "IP_ADD_MEMBERSHIP option" << endl;

        ASSERT(option_len == sizeof(struct ip_mreq));

        struct ip_mreq *mreq = (struct ip_mreq*)option_value;

        IPAddress mcast_addr = IPAddress(ntohl(mreq->imr_multiaddr.s_addr));
        IPAddress mcast_if = IPAddress(ntohl(mreq->imr_interface.s_addr));

        EV << "interface=" << mcast_if << " multicast address=" << mcast_addr << endl;

        RoutingTable *rt = RoutingTableAccess().get();

        InterfaceEntry *ie = rt->interfaceByAddress(mcast_if);

        ASSERT(ie);

        if(!ie->isMulticast())
        {
            EV << "allowing multicast on " << ie->name() << endl;
            ie->setMulticast(true);
        }

        std::vector<IPAddress> mcast_grps = ie->ipv4()->multicastGroups();

        if(find(mcast_grps.begin(), mcast_grps.end(), mcast_addr) == mcast_grps.end())
        {
            EV << "adding multicast membership for group " << mcast_addr << " on " << ie->name() << endl;
            mcast_grps.push_back(mcast_addr);
            ie->ipv4()->setMulticastGroups(mcast_grps);
        }

        bool found = false;

        for(int i = 0; i < rt->numRoutingEntries(); i++)
        {
            RoutingEntry *re = rt->routingEntry(i);

            if(!re->host.equals(mcast_addr))
                continue;

            if(re->interfacePtr != ie)
                continue;

            found = true;
        }

        if(!found)
        {
            EV << "adding multicast route for group " << mcast_addr << " on " << ie->name() << endl;

            RoutingEntry *re = new RoutingEntry();
            re->host = mcast_addr;
            re->gateway = IPAddress::UNSPECIFIED_ADDRESS;
            re->netmask = IPAddress::ALLONES_ADDRESS;
            re->type = RoutingEntry::DIRECT;
            re->metric = 1;
            re->interfacePtr = ie;
            re->interfaceName = ie->name();
            re->source = RoutingEntry::IFACENETMASK; // ???
            rt->addRoutingEntry(re);
        }

        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_HDRINCL)
    {
        EV << "IP_HDRINCL option" << endl;

        ASSERT(option_len == sizeof(int));

        dm->getRawSocket(socket)->setHdrincl(*(int*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_PKTINFO)
    {
        EV << "IP_PKTINFO option" << endl;

        ASSERT(option_len == sizeof(int));

        dm->getRawSocket(socket)->setPktinfo(*(int*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_LOOP)
    {
        EV << "IP_MULTICAST_LOOP option" << endl;

        ASSERT(option_len == sizeof(u_char));

        dm->getRawSocket(socket)->setMulticastLoop(*(u_char*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_TTL)
    {
        EV << "IP_MULTICAST_TTL option" << endl;

        ASSERT(option_len == sizeof(u_char));

        dm->getRawSocket(socket)->setMulticastTtl(*(u_char*)option_value);
        return 0;
    }

    ASSERT(false);
}

int oppsim_socket(int domain, int type, int protocol)
{
    Daemon *dm = current_module; //DAEMON

    EV << "socket: domain=" << domain << " type=" << type << " protocol=" << protocol << endl;

    if(domain == AF_INET && type == SOCK_STREAM && protocol == 0)
    {
        return dm->createTcpSocket();
    }

    if(domain == AF_NETLINK && type == SOCK_RAW && protocol == NETLINK_ROUTE)
    {
        return dm->createNetlinkSocket();
    }

    if(domain == AF_INET && type == SOCK_DGRAM && protocol == 0)
    {
        return dm->createUdpSocket();
    }

    if(domain == AF_INET && type == SOCK_RAW && protocol == IPPROTO_OSPFIGP)
    {
        return dm->createRawSocket(protocol);
    }

    ASSERT(false);
}

void oppsim_openlog(const char *ident, int logopt, int facility)
{
    EV << "openlog: ident=" << ident << " logopt=" << logopt << " facility=" << facility << endl;

    EV << "dummy, do nothing" << endl;
}

void oppsim_closelog()
{
    ASSERT(false);
}

void oppsim_vsyslog(int priority, const char *format, va_list ap)
{
    ASSERT(false);
}

int oppsim_fcntl(int fildes, int cmd, ...)
{
    EV << "fcntl: fildes=" << fildes << " cmd=" << cmd << endl;

    EV << "fcntl does currently nothing" << endl;

    return 0;
}

int oppsim_getsockname(int socket, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "getsockname: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    Netlink *nl = dm->getIfNetlinkSocket(socket);
    if(nl)
    {
        ASSERT(*address_len == sizeof(sockaddr_nl));

        struct sockaddr_nl *nladdr = (sockaddr_nl*)address;

        nladdr->nl_pid = nl->local.nl_pid;
        nladdr->nl_family = nl->local.nl_family;
        nladdr->nl_pad = nl->local.nl_pad;
        nladdr->nl_groups = nl->local.nl_groups;

        return 0;
    }

    ASSERT(false);
}

int getIpHeader(SocketMsg *srcMsg, IPControlInfo *srcInfo, void* &dstPtr, int &dstLen)
{
    int header_length = sizeof(struct ip);

    ASSERT(dstLen >= header_length);

    struct ip * iph = (struct ip *)dstPtr;

    iph->ip_v = 4;
    iph->ip_hl = header_length / sizeof(u_int32_t);
    iph->ip_tos = srcInfo->diffServCodePoint();
    iph->ip_len = htons(srcMsg->byteLength() + header_length);
    iph->ip_ttl = srcInfo->timeToLive();
    iph->ip_p = srcInfo->protocol();
    iph->ip_src.s_addr = htonl(srcInfo->srcAddr().getInt());
    iph->ip_dst.s_addr = htonl(srcInfo->destAddr().getInt());

    iph->ip_id = 0; // ???
    iph->ip_off = 0; // ???
    iph->ip_sum = 0; // ???

    dstPtr = (char*)dstPtr + header_length;
    dstLen -= header_length;

    return header_length;
}

ssize_t receive_stream(int socket, void *buf, size_t nbyte)
{
    Daemon *dm = current_module; //DAEMON

    while(dm->isBlocking(socket) && !dm->getSocketMessage(socket, false))
    {
        dm->receiveAndHandleMessage(0.0);
    }

    int bread = 0;

    while(nbyte > 0)
    {
        cMessage *m = dm->getSocketMessage(socket, false);
        if(!m)
            break;

        SocketMsg *msg = check_and_cast<SocketMsg*>(m);

        int datalen = msg->getDataArraySize();
        bool empty = false;

        if(datalen > nbyte)
        {
            // not enough space in buffer
            datalen = nbyte;
        }
        else
        {
            // no more data in this message
            empty = true;
        }

        msg->copyDataToBuffer(buf, datalen);

        buf = (char*)buf + datalen;
        nbyte -= datalen;
        bread += datalen;

        if(empty)
        {
            delete dm->getSocketMessage(socket, true);
        }
        else
        {
            msg->removePrefix(datalen);
        }
    }

    ASSERT(bread > 0);

    return bread;
}

ssize_t receive_raw(int socket, struct msghdr *message, int flags)
{
    Daemon *dm = current_module; //DAEMON

    ASSERT(message->msg_iovlen == 1);

    bool peek = HASFLAG(flags, MSG_PEEK);

    ASSERT(!flags || flags == MSG_PEEK);

    SocketMsg *msg = check_and_cast<SocketMsg*>(dm->getSocketMessage(socket, !peek));
    IPControlInfo *ipControlInfo = check_and_cast<IPControlInfo*>(msg->controlInfo());

    if(message->msg_name)
    {
        ASSERT(message->msg_namelen == sizeof(sockaddr_in));
        struct sockaddr_in *inaddr = (sockaddr_in*)message->msg_name;
        inaddr->sin_family = AF_INET;
        inaddr->sin_port = htons(0);
        inaddr->sin_addr.s_addr = htonl(ipControlInfo->srcAddr().getInt());
    }

    message->msg_flags = 0;

    int length = message->msg_iov[0].iov_len;
    void *buffer = message->msg_iov[0].iov_base;
    int bread = 0;

    if(dm->getRawSocket(socket)->getHdrincl())
    {
        bread += getIpHeader(msg, ipControlInfo, buffer, length);
    }

    unsigned int datalen = msg->getDataArraySize();

    if(datalen > length)
    {
        // available data won't fit into the buffer
        datalen = length;
        message->msg_flags |= MSG_TRUNC;
    }

    msg->copyDataToBuffer(buffer, datalen);
    bread += datalen;

    // return control data

    if(message->msg_control)
    {
        // IP_PKTINFO

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(message);
        ASSERT(cmsg == message->msg_control);

        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = sizeof(int) + sizeof(struct cmsghdr);

        int interfaceId = ipControlInfo->interfaceId();
        ASSERT(interfaceId >= 0);
        *(int*)CMSG_DATA(cmsg) = interfaceId;

        EV << "IP_PKTINFO set to " << interfaceId << endl;

        ASSERT(message->msg_controllen >= cmsg->cmsg_len);

        message->msg_controllen = cmsg->cmsg_len;
    }

    ASSERT(bread > 0);

    return bread;
}

ssize_t oppsim_recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "recvfrom: socket=" << socket << " buffer=" << buffer << " length=" << length << " flags=" <<
            flags << " address=" << address << " address_len=" << address_len << endl;

    RawSocket *raw = dm->getIfRawSocket(socket);
    if(raw)
    {
        struct msghdr msg;
        struct iovec iov;

        msg.msg_name = address;
        msg.msg_namelen = address? *address_len: 0;
        msg.msg_iovlen = 1;
        msg.msg_iov = &iov;
        msg.msg_iov[0].iov_base = buffer;
        msg.msg_iov[0].iov_len = length;
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags = 0;

        int ret = receive_raw(socket, &msg, flags);

        if(address)
            *address_len = msg.msg_namelen;

        return ret;
    }

    UDPSocket *udp = dm->getIfUdpSocket(socket);
    if(udp)
    {
        ASSERT(!flags);
        ASSERT(*address_len == sizeof(sockaddr_in));

        SocketMsg *msg = check_and_cast<SocketMsg*>(dm->getSocketMessage(socket, true));

        UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo*>(msg->controlInfo());

        struct sockaddr_in *inaddr = (sockaddr_in*)address;
        inaddr->sin_family = AF_INET;
        inaddr->sin_port = htons(udpControlInfo->srcPort());
        inaddr->sin_addr.s_addr = htonl(udpControlInfo->srcAddr().get4().getInt());

        unsigned int datalen = msg->getDataArraySize();

        if(length < datalen)
        {
            EV << "warning: discarding " << (datalen - length) << " bytes from this message" << endl;

            datalen = length;
        }

        msg->copyDataToBuffer(buffer, datalen);

        delete msg;

        ASSERT(datalen > 0);

        return datalen;
    }

    ASSERT(false);
}

ssize_t nl_request(int socket, const void *message, size_t length, int flags)
{
    Daemon *dm = current_module; //DAEMON

    EV << "netlink request, process immediately and store result" << endl;

    Netlink *nl = dm->getNetlinkSocket(socket);

    struct req_t
    {
        struct nlmsghdr nlh;
        struct rtgenmsg g;
    } *req;

    req = (struct req_t*)message;

    ASSERT(req->nlh.nlmsg_len >= sizeof(req_t));
    ASSERT(req->nlh.nlmsg_pid == 0);

    EV << "seq=" << req->nlh.nlmsg_seq << " type=" << req->nlh.nlmsg_type << " family=" <<
            (int)req->g.rtgen_family << " flags=" << req->nlh.nlmsg_flags << endl;

    // process request

    if(req->nlh.nlmsg_type == RTM_GETLINK && req->g.rtgen_family == AF_PACKET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            EV << "RTM_GETLINK dump" << endl;

            nl->appendResult(nl->listInterfaces());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_GETADDR && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            EV << "RTM_GETADDR dump" << endl;

            nl->appendResult(nl->listAddresses());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_GETROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            EV << "RTM_GETROUTE dump" << endl;

            nl->appendResult(nl->listRoutes());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_DELROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK))
        {
            EV << "RTM_DELROUTE create request" << endl;

            RoutingEntry* re = nl->route_command(req->nlh.nlmsg_type, (ret_t*)message);

            nl->appendResult(nl->listRoutes(re));

            return 0;
        }
    }


    if(req->nlh.nlmsg_type == RTM_NEWROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK))
        {
            EV << "RTM_NEWROUTE create request" << endl;

            RoutingEntry* re = nl->route_command(req->nlh.nlmsg_type, (ret_t*)message);

            nl->appendResult(nl->listRoutes(re));

            return 0;
        }
    }

    ASSERT(false);
}

ssize_t oppsim_sendmsg(int socket, const struct msghdr *message, int flags)
{
    Daemon *dm = current_module; //DAEMON

    EV << "sendmsg: socket=" << socket << " msghdr=" << message << " flags=" << flags << endl;

    if(dm->getIfNetlinkSocket(socket))
    {
        ASSERT(message->msg_iovlen == 1);
        ASSERT(flags == 0);

        EV << "message->msg_flags=" << message->msg_flags << endl;

        return nl_request(socket, message->msg_iov[0].iov_base, message->msg_iov[0].iov_len, message->msg_flags);
    }

    RawSocket *raw = dm->getIfRawSocket(socket);
    if(raw)
    {
        return raw->send(message, flags);
    }

    ASSERT(false);
}

ssize_t oppsim_sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "sendto: socket=" << socket << " message=" << message << " length=" << length << " flags=" << flags <<
            " dest_addr=" << dest_addr << " dest_len=" << dest_len << endl;

    if(dm->getIfNetlinkSocket(socket))
    {
        // make sure it is for us (kernel)
        ASSERT(dest_len == sizeof(struct sockaddr_nl));
        struct sockaddr_nl * nladdr = (sockaddr_nl*)dest_addr; // destination socket
        ASSERT(nladdr->nl_family == AF_NETLINK);
        ASSERT(nladdr->nl_pad == 0);
        ASSERT(nladdr->nl_pid == 0);
        ASSERT(nladdr->nl_groups == 0);

        struct req_t
        {
            struct nlmsghdr nlh;
            struct rtgenmsg g;
        };

        ASSERT(length == sizeof(req_t));

        return nl_request(socket, message, length, flags);
    }

    UDPSocket *udp = dm->getIfUdpSocket(socket);
    if(udp)
    {
        EV << "udp socket" << endl;

        ASSERT(dest_addr);
        ASSERT(dest_addr->sa_family == AF_INET);
        ASSERT(dest_len == sizeof(sockaddr_in));

        struct sockaddr_in *inaddr = (sockaddr_in*)dest_addr;

        IPAddress destAddr = IPAddress(ntohl(inaddr->sin_addr.s_addr));
        int port = ntohs(inaddr->sin_port);

        EV << "destAddr=" << destAddr << " port=" << port << endl;

        SocketMsg *msg = new SocketMsg("data");

        ASSERT(length > 0);

        msg->setDataFromBuffer(message, length);

        msg->setByteLength(length);

        udp->sendTo(msg, destAddr, port);

        return length;
    }

    ASSERT(false);
}

ssize_t oppsim_recvmsg(int socket, struct msghdr *message, int flags)
{
    Daemon *dm = current_module; //DAEMON

    EV << "recvmsg: socket=" << socket << " message=" << message << " flags=" << flags << endl;

    Netlink* nl = dm->getIfNetlinkSocket(socket);
    if(nl)
    {
        EV << "reading netlink result" << endl;

        ASSERT(message);

        return nl->getNextResult().copyout(message);
    }

    if(dm->getIfRawSocket(socket))
    {
        EV << "reading from raw socket" << endl;

        return receive_raw(socket, message, flags);
    }

    ASSERT(false);
}

int oppsim_stat(const char *path, struct stat *buf)
{
    ASSERT(false);
    return -1;
}

FILE* oppsim_fopen(const char * filename, const char * mode)
{
    Daemon *dm = current_module; //DAEMON

    EV << "fopen: filename=" << filename << " mode=" << mode << endl;

    ASSERT(*filename == '/');

    // translate "/etc/quagga/ripd.conf" to "_etc_quagga_ripd.conf"
    char *filename2 = strdup(filename);
    for (char *s = filename2; *s; s++)
        if (*s == '/')
            *s = '_';
    std::string rpath = (dm->getrootprefix() + "/" + filename2);
    free(filename2);

    EV << "real filename=" << rpath << endl;

    return fopen(rpath.c_str(), mode);
}

int oppsim_getpagesize()
{
    EV << "getpagesize: we use 8192" << endl;
    return 8192;
}

int oppsim_open(const char *path, int oflag, ...)
{
    Daemon *dm = current_module; //DAEMON

    EV << "open: path=" << path << " oflag=" << oflag << endl;

    ASSERT(*path == '/');

    std::string rpath = (dm->getrootprefix() + path);

    EV << "real path=" << rpath << endl;

    if(oflag == (O_RDWR | O_CREAT))
    {
        va_list ap;
        va_start(ap, oflag);
        mode_t mode = va_arg(ap, mode_t);
        va_end(ap);

        EV << "mode=" << mode << endl;
        EV << "ignoring mode option" << endl;

        return dm->createStream(rpath.c_str(), "r+");
    }

    // this either
    ASSERT(false);
}

int oppsim_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout)
{
    Daemon *dm = current_module; //DAEMON

    EV << "select: nfds=" << nfds << " readfds=" << readfds << " writefds=" << writefds << " errorfds=" <<
            errorfds << " timeout=" << timeout << endl;

    int success = 0;
    
    double limit;
    if(timeout)
    {
        limit = simulation.simTime() + (double)timeout->tv_sec + (double)timeout->tv_usec/1000000;
        ASSERT(limit > simulation.simTime());
    }

    while(!success)
    {
        EV << "select: checking descriptors:" << endl;
        
        // first see if any descriptor is ready
        for(int i = 0; i < nfds; i++)
        {
            if(FD_ISSET(i, readfds))
            {
                EV << "read " << i;
    
                if(dm->getSocketMessage(i))
                {
                    EV << " (ready)" << endl;
                    ++success;
                    continue;
                }
                else if(dm->getIfTcpSocket(i) && dm->hasQueuedConnections(i))
                {
                    EV << " (ready)" << endl;
                    ++success;
                    continue;
                }
                else
                {
                    EV << endl;
                }
            }
    
            if(FD_ISSET(i, writefds))
            {
                EV << "write " << i << " (write is always ready)" << endl;
                ++success;
                continue;
            }
    
            if(FD_ISSET(i, errorfds))
            {
                EV << "error " << i << endl;
    
                // exceptions not supported at the moment
            }
        }

        // some descriptors are ready, we're done
        if(success) break;
        
        EV << "select" << endl;
        
        if(timeout)
        {
            double d = limit - simulation.simTime();
            if(!dm->receiveAndHandleMessage(d))
               return 0; // timeout received before any event arrived
        }
        else
        {
            // with no timeout
            dm->receiveAndHandleMessage(0.0);
        }
    }

    ASSERT(success);

    EV << "there are " << success << " sockets ready" << endl;

    for(int i = 0; i < nfds; i++)
    {
        if(FD_ISSET(i, readfds))
        {
            bool active = false;

            if(dm->getSocketMessage(i))
            {
                ev << " active (data ready)" << endl;
                active = true;
            }

            if(dm->getIfTcpSocket(i) && dm->hasQueuedConnections(i))
            {
                ev << " active (incomming connection)" << endl;
                active = true;
            }

            if(!active)
            {
                //ev << " clear" << endl;

                FD_CLR(i, readfds);
            }
        }

        // write is always ready, don't touch writefds
    }

    // exceptions not supported yet
    FD_ZERO(errorfds);

    return success;
}

int oppsim_close(int fildes)
{
    Daemon *dm = current_module; //DAEMON

    EV << "close: fildes=" << fildes << endl;

    FILE *stream = dm->getIfStream(fildes);
    if(stream)
    {
        EV << "closing file descriptor" << endl;

        dm->closeStream(fildes);

        return 0;
    }

    UDPSocket *udp = dm->getIfUdpSocket(fildes);
    if(udp)
    {
        EV << "closing UDP socket" << endl;

        dm->closeSocket(fildes);

        return 0;
    }
    
    TCPSocket *tcp = dm->getIfTcpSocket(fildes);
    if(tcp)
    {
        EV << "closing TCP socket" << endl;
        
        dm->closeSocket(fildes);
        
        return 0;
    }

    // closing RAW/NETLINK not implemented
    ASSERT(false);
}

void oppsim_perror(const char *s)
{
    EV << "perror: " << s << ": " << strerror(GlobalVars_errno()) << endl;

    ASSERT(false);
}

ssize_t oppsim_writev(int fildes, const struct iovec *iov, int iovcnt)
{
    EV << "writev: fildes=" << fildes << " iov=" << iov << " iovcnt=" << iovcnt << endl;

    Daemon *dm = current_module; //DAEMON

    FILE *stream = dm->getStream(fildes);

    int written = 0;

    for(int i = 0; i < iovcnt; i++)
    {
        ASSERT(iov[i].iov_base);
        written += fwrite(iov[i].iov_base, 1, iov[i].iov_len, stream);
    }

    return written;
}

ssize_t oppsim_write(int fildes, const void *buf, size_t nbyte)
{
    Daemon *dm = current_module; //DAEMON;

    EV << "write: fildes=" << fildes << " buf=" << buf << " nbyte=" << nbyte << endl;

    TCPSocket *tcp = dm->getIfTcpSocket(fildes);
    if(tcp)
    {
        EV << "write into tcp socket" << endl;

        SocketMsg *msg = new SocketMsg("data");

        msg->setDataFromBuffer(buf, nbyte);
        msg->setByteLength(nbyte);

        tcp->send(msg);

        return nbyte;
    }

    FILE *stream = dm->getIfStream(fildes);
    if(stream)
    {
        EV << "regular file descriptor, use fwrite" << endl;

        return fwrite(buf, 1, nbyte, stream);
    }

    // write to RAW/NETLINK/UDP shouldn't be used
    ASSERT(false);
}

int oppsim_listen(int socket, int backlog)
{
    Daemon *dm = current_module; //DAEMON

    EV << "listen: socket=" << socket << " backlog=" << backlog << endl;

    dm->getTcpSocket(socket)->listen();

    return 0;
}


int oppsim_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "bind: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    ASSERT(address);

    if(address->sa_family == AF_INET)
    {
        const struct sockaddr_in *inaddr = (sockaddr_in*)address;

        int port = ntohs(inaddr->sin_port);
        int addr = ntohl(inaddr->sin_addr.s_addr);

        EV << "binding to address=" << IPAddress(addr) << " port=" << port << endl;

        TCPSocket *tcp = dm->getIfTcpSocket(socket);
        if(tcp)
        {
            tcp->bind(IPAddress(addr), port);

            return 0;
        }

        UDPSocket *udp = dm->getIfUdpSocket(socket);
        if(udp)
        {
            udp->setUserId(socket);
            udp->bind(IPAddress(addr), port);

            return 0;
        }

        ASSERT(false);
    }

    if(address->sa_family == AF_NETLINK)
    {
        const struct sockaddr_nl *nladdr = (sockaddr_nl*)address;

        int pid = nladdr->nl_pid;

        if(pid == 0)
        {
            pid = oppsim_getpid();
            EV << "autogenerating nl_pid" << endl;
        }

        dm->getNetlinkSocket(socket)->bind(pid);

        EV << "binding to nl_pid=" << pid << endl;

        return 0;
    }

    ASSERT(false);
}

int oppsim_accept(int socket, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "accept: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    int csocket = dm->acceptTcpSocket(socket);
    ASSERT(csocket != -1);

    if(address)
    {
        ASSERT(address_len);
        ASSERT(*address_len == sizeof(sockaddr_in));
        struct sockaddr_in *inaddr = (sockaddr_in*)address;

        TCPSocket *tcp = dm->getTcpSocket(csocket);
        inaddr->sin_family = AF_INET;
        inaddr->sin_port = htons(tcp->remotePort());
        inaddr->sin_addr.s_addr = htonl(tcp->remoteAddress().get4().getInt());
    }

    return csocket;
}

int oppsim_getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len)
{
    ASSERT(false);
    return -1;
}

int oppsim_ioctl(int fildes, int request, ...)
{
    ASSERT(false);
    return -1;
}

ssize_t oppsim_read(int fildes, void *buf, size_t nbyte)
{
    Daemon *dm = current_module; //DAEMON;

    EV << "read: fildes=" << fildes << " buf=" << buf << " nbyte=" << nbyte << endl;

    if(dm->getIfTcpSocket(fildes))
    {
        EV << "reading from TCP socket" << endl;

        return receive_stream(fildes, buf, nbyte);
    }

    // read from STEAM/RAW/NETLINK/UDP shouldn't be used
    ASSERT(false);
}

int oppsim_uname(struct utsname *name)
{
    ASSERT(false);
    return -1;
}

int oppsim_mkstemp(char *tmpl)
{
    ASSERT(false);
    return -1;
}

int oppsim_chmod(const char *path, mode_t mode)
{
    ASSERT(false);
    return -1;
}

int oppsim_unlink(const char *path)
{
    ASSERT(false);
    return -1;
}

int oppsim_link(const char *path1, const char *path2)
{
    ASSERT(false);
    return -1;
}

char *oppsim_getcwd(char *buf, size_t size)
{
    Daemon *dm = current_module; //DAEMON

    EV << "getcwd: buf=" << (void*)buf << " size=" << size << endl;

    std::string cwd = dm->getcwd();

    const char *cwdstr = cwd.c_str();

    ASSERT(strlen(cwdstr) < size);

    strncpy(buf, cwdstr, size);

    EV << "cwd=" << buf << endl;

    return buf;
}

int oppsim_chdir(const char *path)
{
    ASSERT(false);
    return -1;
}

struct passwd *oppsim_getpwnam(const char *name)
{
    Daemon *dm = current_module; //DAEMON

    EV << "getpwnam: name=" << name << endl;

    return &dm->pwd_entry;
}

struct group *oppsim_getgrnam(const char *name)
{
    Daemon *dm = current_module; //DAEMON

    EV << "getgrnam: name=" << name << endl;

    return &dm->grp_entry;
}

int oppsim_setgroups(size_t size, const gid_t *list)
{
    ASSERT(false);
    return -1;
}

int oppsim_connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    Daemon *dm = current_module; //DAEMON

    EV << "connect: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    struct sockaddr_in *inaddr = (sockaddr_in*)address;
    ASSERT(address_len == sizeof(*inaddr));
    IPAddress destAddr = IPAddress(ntohl(inaddr->sin_addr.s_addr));

    EV << "destAddress=" << destAddr << endl;

    TCPSocket *tcp = dm->getIfTcpSocket(socket);

    if(tcp->localAddress().isUnspecified())
    {
        // XXX FIXME this should be probably fixed in TCP layer
        // XXX FIXME this hack doesn't work if client is already bound (with 0.0.0.0:some_port)

        IPAddress localAddr;

        RoutingTable *rt = check_and_cast<RoutingTable*>(dm->parentModule()->submodule("routingTable"));
        if(rt->localDeliver(destAddr))
        {
            localAddr = destAddr;
        }
        else
        {
            InterfaceEntry *ie = rt->interfaceForDestAddr(destAddr);
            ASSERT(ie!=NULL);
            localAddr = ie->ipv4()->inetAddress();
        }

        EV << "autoassigning local address=" << localAddr << endl;

        tcp->bind(localAddr, -1);
    }

    tcp->connect(destAddr, ntohs(inaddr->sin_port));

    EV << "connect to destAddr=" << destAddr << " port=" << ntohs(inaddr->sin_port) << endl;
    
    if(dm->isBlocking(socket))
    {
        while(true)
        {
            EV << "connect" << endl;
            
            dm->receiveAndHandleMessage(0.0);
            
            if(tcp->state() == tcp->CONNECTED)
                break;
        } 
        
        EV << "connection fully established" << endl;
    }

    return 0;
}

int oppsim_getpeername(int socket, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *d = current_module; //DAEMON
    
    EV << "getpeername:  socket=" << socket << endl;
    
    TCPSocket *tcp = d->getTcpSocket(socket);
    
    ASSERT(*address_len >= sizeof(struct sockaddr_in));
    
    struct sockaddr_in *inaddr = (struct sockaddr_in*)address;
    
    inaddr->sin_family = AF_INET;
    inaddr->sin_port = htons(tcp->remotePort());
    inaddr->sin_addr.s_addr = htonl(tcp->remoteAddress().get4().getInt());
    
    *address_len = sizeof(struct sockaddr_in);

    return 0;
}

void oppsim_abort()
{
    ASSERT(false);
}

int oppsim_sigfillset(sigset_t *set)
{
    memset(set, 0xff, sizeof(sigset_t));
    return 0;
}

void oppsim__exit(int status)
{
    ASSERT(false);
}

int oppsim_sigaction(int sig, const struct_sigaction *act, struct_sigaction *oact)
{
    Daemon *dm = current_module; //DAEMON

    EV << "sigaction: sig=" << sig << " act=" << act << " oact=" << oact << endl;

    struct_sigaction *sptr = dm->sigactionimpl(sig);

    if(oact)
        memcpy(oact, sptr, sizeof(*oact));

    if(act)
        memcpy(sptr, act, sizeof(*sptr));

    return 0;
}

char *oppsim_getenv(const char *name)
{
    return getenv(name);
}

inline bool bigendian()
{
    static int a = 255;
    return *(char *)(&a) == 0;
}

u_long oppsim_htonl(u_long hostlong)
{
    if (bigendian())
        return hostlong;
    else
        return ((hostlong & 0xff)<<24) | ((hostlong & 0xff00)<<8) | ((hostlong & 0xff0000)>>8) | ((hostlong & 0xff000000)>>24);
}

u_short oppsim_htons(u_short hostshort)
{
    if (bigendian())
        return hostshort;
    else
        return ((hostshort & 0xff)<<8) | ((hostshort & 0xff00)>>8);
}

char *oppsim_inet_ntoa(struct in_addr in)
{
    static char buf[32];
    sprintf(buf,"%d.%d.%d.%d", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2,
                               in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
    return buf;
}

u_long oppsim_ntohl(u_long netlong)
{
    if (bigendian())
        return netlong;
    else
        return ((netlong & 0xff)<<24) | ((netlong & 0xff00)<<8) | ((netlong & 0xff0000)>>8) | ((netlong & 0xff000000)>>24);
}

u_short oppsim_ntohs(u_short netshort)
{
    if (bigendian())
        return netshort;
    else
        return ((netshort & 0xff)<<8) | ((netshort & 0xff00)>>8);
}

unsigned long oppsim_inet_addr(const char *str)
{
    // should return -1 on error
    if (!IPAddress::isWellFormed(str))
        return -1;
    return oppsim_htonl(IPAddress(str).getInt());
}

int oppsim_inet_aton(const char *str, struct in_addr *addr)
{
    // should return 1 on success, 0 on error
    if (!IPAddress::isWellFormed(str))
        return 0;
    addr->s_addr = oppsim_htonl(IPAddress(str).getInt());
    return 1;
}

int oppsim_inet_pton(int af, const char *strptr, void *addrptr)
{
    // should return 1 on success, 0 if not parsable, -1 otherwise
    if (af==AF_INET) {
        struct in_addr *inaddr = (in_addr *)addrptr;
        return oppsim_inet_aton(strptr, inaddr);
    } else {
        opp_error("oppsim_inet_pton: address family not supported");
    }
}

char *oppsim_inet_ntop(int af, const void *src, char *dst, size_t size)
{
    if (af==AF_INET) {
        ASSERT(size>=16);
        struct in_addr& in = *(in_addr *)src;
        sprintf(dst,"%d.%d.%d.%d", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2,
                                   in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
        return dst;
    } else {
        opp_error("oppsim_inet_ntop: address family not supported");
    }
}

