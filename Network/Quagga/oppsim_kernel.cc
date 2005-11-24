
#include <omnetpp.h>

#include "zebra_env.h"

#include "oppsim_kernel.h"

#include "Daemon.h"

#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "Socket_m.h"

#include "UDPControlInfo_m.h"
#include "TCPCommand_m.h"
#include "IPControlInfo_m.h"

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

#endif

struct servent *oppsim_getservbyname(const char *name, const char *proto)
{
    // XXX FIXME implement custom (fixed) mapping
    return getservbyname(name, proto);
}


long int oppsim_random(void)
{
    return intrand(ULONG_MAX);
}

//

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
    return -1;
}

uid_t oppsim_geteuid(void)
{
    int euid = DAEMON->euid;

    ev << "geteuid: return " << euid << endl;

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
    Daemon *libm = DAEMON;

    ev << "setsockopt: socket=" << socket << " level=" << level << " option_name=" << option_name <<
            " option_value=" << option_value << " option_len=" << option_len << endl;

    ASSERT(libm->issocket(socket));

    if(level == SOL_SOCKET && option_name == SO_RCVBUF)
    {
        ev << "SO_RCVBUF option, ignore" << endl;
        return 0;
    }

    if(level == SOL_SOCKET && option_name == SO_REUSEADDR)
    {
        ev << "SO_REUSEADDR option, ignore" << endl;
        return 0;
    }

    if(level == SOL_SOCKET && option_name == SO_BROADCAST)
    {
        ev << "SO_BROADCAST option, ignore" << endl;
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_IF)
    {
        // if this assert fails, make sure Zebra is not compiled with struct ip_mreqn here
        ASSERT(option_len == sizeof(struct in_addr));

        const struct in_addr *iaddr  = (in_addr*)option_value;

        IPAddress addr = IPAddress(ntohl(iaddr->s_addr));
        ev << "IP_MULTICAST_IF: " << addr << endl;

        if(libm->isudpsocket(socket))
        {
            InterfaceEntry *ie = RoutingTableAccess().get()->interfaceByAddress(addr);
            ASSERT(ie);
            libm->getudpsocket(socket)->setMulticastInterface(ie->outputPort());

            return 0;
        }
        else if(libm->israwsocket(socket))
        {
            libm->getrawsocket(socket)->setMulticastIf(addr);

            return 0;
        }

        ASSERT(false);
    }

    if(level == 0 && option_name == IP_ADD_MEMBERSHIP)
    {
        ev << "IP_ADD_MEMBERSHIP option, ignore" << endl;
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_HDRINCL)
    {
        ev << "IP_HDRINCL option" << endl;

        ASSERT(libm->israwsocket(socket));
        ASSERT(option_len == sizeof(int));

        libm->getrawsocket(socket)->setHdrincl(*(int*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_PKTINFO)
    {
        ev << "IP_PKTINFO option" << endl;

        ASSERT(libm->israwsocket(socket));
        ASSERT(option_len == sizeof(int));

        libm->getrawsocket(socket)->setPktinfo(*(int*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_LOOP)
    {
        ev << "IP_MULTICAST_LOOP option" << endl;

        ASSERT(libm->israwsocket(socket));
        ASSERT(option_len == sizeof(u_char));

        libm->getrawsocket(socket)->setMulticastLoop(*(u_char*)option_value);
        return 0;
    }

    if(level == IPPROTO_IP && option_name == IP_MULTICAST_TTL)
    {
        ev << "IP_MULTICAST_TTL option" << endl;

        ASSERT(libm->israwsocket(socket));
        ASSERT(option_len == sizeof(u_char));

        libm->getrawsocket(socket)->setMulticastTtl(*(u_char*)option_value);
        return 0;
    }

    ASSERT(false);
}

int oppsim_socket(int domain, int type, int protocol)
{
    Daemon *libm = DAEMON;

    ev << "socket: domain=" << domain << " type=" << type << " protocol=" << protocol << endl;

    if(domain == AF_INET && type == SOCK_STREAM && protocol == 0)
    {
        return libm->createTcpSocket();
    }

    if(domain == AF_NETLINK && type == SOCK_RAW && protocol == NETLINK_ROUTE)
    {
        return libm->createNetlinkSocket();
    }

    if(domain == AF_INET && type == SOCK_DGRAM && protocol == 0)
    {
        return libm->createUdpSocket();
    }

    if(domain == AF_INET && type == SOCK_RAW && protocol == IPPROTO_OSPFIGP)
    {
        return libm->createRawSocket(protocol);
    }

    ASSERT(false);
}

void oppsim_openlog(const char *ident, int logopt, int facility)
{
    ev << "openlog: ident=" << ident << " logopt=" << logopt << " facility=" << facility << endl;

    ev << "dummy, do nothing" << endl;
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
    Daemon *libm = DAEMON;

    ev << "fcntl: fildes=" << fildes << " cmd=" << cmd << endl;

    if(libm->issocket(fildes))
    {
        ev << "socket operation, ignore" << endl;
        return 0;
    }

    // XXX we would have to parse cmd and fetch appropriate arguments etc

    ev << "regular descriptor, ignore" << endl;
    return 0;
}

int oppsim_getsockname(int socket, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *libm = DAEMON;

    ev << "getsockname: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    if(libm->istcpsocket(socket))
    {
        ASSERT(*address_len == sizeof(sockaddr_in));

        ASSERT(false);
    }

    if(libm->isnlsocket(socket))
    {
        Netlink *nl = libm->getnlsocket(socket);

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

ssize_t receive_raw(int socket, struct msghdr *message, int flags)
{
    Daemon *libm = DAEMON;

    ASSERT(message->msg_iovlen == 1);

    bool peek = ((flags & MSG_PEEK) == MSG_PEEK);

    ASSERT(!flags || flags == MSG_PEEK);

    SocketMsg *msg = check_and_cast<SocketMsg*>(libm->getqueuetail(socket, false));
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

    unsigned int datalen = msg->getDataArraySize();

    int length = message->msg_iov[0].iov_len;
    void *buffer = message->msg_iov[0].iov_base;

    int bread = 0; // bytes successfully read

    if(libm->getrawsocket(socket)->getHdrincl())
    {
        int header_length = sizeof(struct ip);

        struct ip * iph = (struct ip *)buffer;

        iph->ip_v = 4;
        iph->ip_hl = header_length / sizeof(u_int32_t);
        iph->ip_tos = ipControlInfo->diffServCodePoint();
        iph->ip_len = htons(msg->byteLength() + header_length);
        iph->ip_ttl = ipControlInfo->timeToLive();
        iph->ip_p = ipControlInfo->protocol();
        iph->ip_src.s_addr = htonl(ipControlInfo->srcAddr().getInt());
        iph->ip_dst.s_addr = htonl(ipControlInfo->destAddr().getInt());

        iph->ip_id = 0; // ???
        iph->ip_off = 0; // ???
        iph->ip_sum = 0; // ???

        buffer = (char*)buffer + header_length;
        length -= header_length;
        bread += header_length;
    }

    int todo;

    if(datalen > length)
    {
        // available data won't fit into the buffer

        message->msg_flags |= MSG_TRUNC;

        todo = length;
    }
    else
    {
        // all data will be read successfully

        todo = datalen;
    }

    // copy data to the output buffer

    char *str = (char*)buffer;
    for(int i = 0; i < todo; i++)
    {
        *(str++) = msg->getData(i);
    }

    bread += todo;

    // return control data

    if(message->msg_control)
    {
        // IP_PKTINFO

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(message);
        ASSERT(cmsg == message->msg_control);

        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = sizeof(int) + sizeof(struct cmsghdr);

        int index = ipControlInfo->inputPort();
        ASSERT(index >= 0);
        *(int*)CMSG_DATA(cmsg) = index + 1;

        ev << "IP_PKTINTO set to " << (index + 1) << endl;

        ASSERT(message->msg_controllen >= cmsg->cmsg_len);

        message->msg_controllen = cmsg->cmsg_len;
    }

    //

    if(!peek)
    {
        // remove packet from queue
        delete libm->getqueuetail(socket, true);
    }

    ASSERT(bread > 0);

    return bread;
}

ssize_t oppsim_recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *libm = DAEMON;

    ev << "recvfrom: socket=" << socket << " buffer=" << buffer << " length=" << length << " flags=" <<
            flags << " address=" << address << " address_len=" << address_len << endl;

    if(libm->israwsocket(socket))
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

    if(libm->isudpsocket(socket))
    {
        ASSERT(!flags);
        ASSERT(*address_len == sizeof(sockaddr_in));

        SocketMsg *msg = check_and_cast<SocketMsg*>(libm->getqueuetail(socket, true));

        UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo*>(msg->controlInfo());

        struct sockaddr_in *inaddr = (sockaddr_in*)address;
        inaddr->sin_family = AF_INET;
        inaddr->sin_port = htons(udpControlInfo->srcPort());
        inaddr->sin_addr.s_addr = htonl(udpControlInfo->srcAddr().get4().getInt());

        unsigned int datalen = msg->getDataArraySize();
        int bread = length < datalen? length: datalen;

        char *str = (char*)buffer;
        for(int i = 0; i < bread; i++)
        {
            *(str++) = msg->getData(i);
        }

        if(bread < datalen)
        {
            ev << "warning: discarding " << (datalen - bread) << " bytes from this message" << endl;
        }

        delete msg;

        ASSERT(bread > 0);

        return bread;
    }

    ASSERT(false);
}

ssize_t nl_request(int socket, const void *message, size_t length, int flags)
{
    Daemon *libm = DAEMON;

    ev << "netlink request, process immediately and store result" << endl;

    ASSERT(libm->isnlsocket(socket));

    Netlink *nl = libm->getnlsocket(socket);    // source socket

    struct req_t
    {
        struct nlmsghdr nlh;
        struct rtgenmsg g;
    } *req;

    req = (struct req_t*)message;

    ASSERT(req->nlh.nlmsg_len >= sizeof(req_t));
    ASSERT(req->nlh.nlmsg_pid == 0);

    ev << "seq=" << req->nlh.nlmsg_seq << " type=" << req->nlh.nlmsg_type << " family=" <<
            (int)req->g.rtgen_family << " flags=" << req->nlh.nlmsg_flags << endl;

    // process request

    if(req->nlh.nlmsg_type == RTM_GETLINK && req->g.rtgen_family == AF_PACKET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            ev << "RTM_GETLINK dump" << endl;

            nl->pushResult(nl->listInterfaces());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_GETADDR && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            ev << "RTM_GETADDR dump" << endl;

            nl->pushResult(nl->listAddresses());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_GETROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST))
        {
            ev << "RTM_GETROUTE dump" << endl;

            nl->pushResult(nl->listRoutes());

            return 0;
        }
    }

    if(req->nlh.nlmsg_type == RTM_DELROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK))
        {
            ev << "RTM_DELROUTE create request" << endl;

            RoutingEntry* re = nl->route_command(req->nlh.nlmsg_type, (rtm_request_t*)message);

            nl->pushResult(nl->listRoutes(re));

            return 0;
        }
    }


    if(req->nlh.nlmsg_type == RTM_NEWROUTE && req->g.rtgen_family == AF_INET)
    {
        if(req->nlh.nlmsg_flags == (NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK))
        {
            ev << "RTM_NEWROUTE create request" << endl;

            RoutingEntry* re = nl->route_command(req->nlh.nlmsg_type, (rtm_request_t*)message);

            nl->pushResult(nl->listRoutes(re));

            return 0;
        }
    }

    ASSERT(false);
}

ssize_t oppsim_sendmsg(int socket, const struct msghdr *message, int flags)
{
    Daemon *d = DAEMON;

    ev << "sendmsg: socket=" << socket << " msghdr=" << message << " flags=" << flags << endl;

    if(d->isnlsocket(socket))
    {
        ASSERT(message->msg_iovlen == 1);
        ASSERT(flags == 0);

        ev << "message->msg_flags=" << message->msg_flags << endl;

        return nl_request(socket, message->msg_iov[0].iov_base, message->msg_iov[0].iov_len, message->msg_flags);
    }
    else if(d->israwsocket(socket))
    {
        return d->getrawsocket(socket)->send(message, flags);
    }

    ASSERT(false);
}

ssize_t oppsim_sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    Daemon *libm = DAEMON;

    ev << "sendto: socket=" << socket << " message=" << message << " length=" << length << " flags=" << flags <<
            " dest_addr=" << dest_addr << " dest_len=" << dest_len << endl;

    if(libm->isnlsocket(socket))
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

    if(libm->isudpsocket(socket))
    {
        ev << "udp socket" << endl;

        ASSERT(dest_addr);
        ASSERT(dest_addr->sa_family == AF_INET);
        ASSERT(dest_len == sizeof(sockaddr_in));

        struct sockaddr_in *inaddr = (sockaddr_in*)dest_addr;

        IPAddress destAddr = IPAddress(ntohl(inaddr->sin_addr.s_addr));
        int port = ntohs(inaddr->sin_port);

        ev << "destAddr=" << destAddr << " port=" << port << endl;

        SocketMsg *msg = new SocketMsg("data");

        ASSERT(length > 0);

        msg->setDataArraySize(length);
        for(int i = 0; i < length; i++)
            msg->setData(i, ((char*)message)[i]);

        msg->setLength(length * 8);

        UDPSocket *udp = libm->getudpsocket(socket);
        udp->sendTo(msg, destAddr, port);

        return length;
    }

    ASSERT(false);
}

ssize_t oppsim_recvmsg(int socket, struct msghdr *message, int flags)
{
    Daemon *libm = DAEMON;

    ev << "recvmsg: socket=" << socket << " message=" << message << " flags=" << flags << endl;

    if(libm->isnlsocket(socket))
    {
        ev << "reading netlink result" << endl;

        ASSERT(message);

        return libm->getnlsocket(socket)->shiftResult().copyout(message);
    }
    else if(libm->israwsocket(socket))
    {
        ev << "reading from raw socket" << endl;

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
    Daemon *libm = DAEMON;

    ev << "fopen: filename=" << filename << " mode=" << mode << endl;

    ASSERT(*filename == '/');

    std::string rpath = (libm->getrootprefix() + filename);

    ev << "real filename=" << rpath << endl;

    return fopen(rpath.c_str(), mode);
}

int oppsim_getpagesize()
{
    ev << "getpagesize: we use 8192" << endl;
    return 8192;
}

int oppsim_open(const char *path, int oflag, ...)
{
    Daemon *libm = DAEMON;

    ev << "open: path=" << path << " oflag=" << oflag << endl;

    ASSERT(*path == '/');

    std::string rpath = (libm->getrootprefix() + path);

    ev << "real path=" << rpath << endl;

    if(oflag == (O_RDWR | O_CREAT))
    {
        va_list ap;
        va_start(ap, oflag);
        mode_t mode = va_arg(ap, mode_t);
        va_end(ap);

        ev << "mode=" << mode << endl;

        //ASSERT(mode == LOGFILE_MASK);
        return libm->createFile(rpath.c_str(), "r+");
    }

    // this either
    ASSERT(false);
}

int oppsim_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout)
{
    Daemon *libm = DAEMON;

    ev << "select: nfds=" << nfds << " readfds=" << readfds << " writefds=" << writefds << " errorfds=" <<
            errorfds << " timeout=" << timeout << endl;

    // make sure we wait on our pseudo-sockets only
    for(int i = 0; i < FD_MIN; i++)
    {
        ASSERT(!readfds || !FD_ISSET(i, readfds));
        ASSERT(!writefds || !FD_ISSET(i, writefds));
        ASSERT(!errorfds || !FD_ISSET(i, errorfds));
    }

    // see if any socket is ready

    int success = 0;

    for(int i = FD_MIN; i < nfds; i++)
    {
        if(FD_ISSET(i, readfds))
        {
            ev << "read " << i << endl;

            if(libm->getqueuetail(i))
            {
                ev << "this socket is ready" << endl;
                ++success;
            }
        }

        if(FD_ISSET(i, writefds))
        {
            ev << "write " << i << endl;

            if(true)
            {
                ev << "write socket is always ready" << endl;
                ++success;
            }
        }

        if(FD_ISSET(i, errorfds))
        {
            ev << "error " << i << endl;

            // exceptions not supported at the moment
        }
    }

    if(!success)
    {
        double limit;

        if(timeout)
        {
            limit = simulation.simTime() + (double)timeout->tv_sec + (double)timeout->tv_usec/1000000;
            ASSERT(limit > simulation.simTime());
        }

        cMessage *msg;

        while(true)
        {
            libm->setblocked(true);

            ev << "we have to block ";

            if(timeout)
            {
                ASSERT(limit != simulation.simTime());

                ev << "with timeout " << (limit - simulation.simTime()) << " secs" << endl;
                msg = libm->receive(limit - simulation.simTime());
            }
            else
            {
                ev << "with no timeout" << endl;
                msg = libm->receive();
                ASSERT(msg);
            }

            libm->setblocked(false);

            current_module = DAEMON;
            __activeVars = current_module->varp;

            if(!msg)
            {
                ASSERT(timeout);

                ev << "timeout expired" << endl;

                return 0;
            }
            else
            {
                ev << "received message=" << msg->name() << " while blocked in select" << endl;

                int socket;

                UDPControlInfo *udpControlInfo = dynamic_cast<UDPControlInfo*>(msg->controlInfo());
                TCPCommand *tcpCommand = dynamic_cast<TCPCommand*>(msg->controlInfo());
                IPControlInfo *ipControlInfo = dynamic_cast<IPControlInfo*>(msg->controlInfo());

                if(udpControlInfo)
                {
                    // UDP packet

                    ASSERT(!strcmp(msg->name(), "data"));

                    socket = udpControlInfo->userId();

                    ASSERT(socket >= 0);

                    libm->enqueue(socket, msg);
                }
                else if(tcpCommand)
                {
                    // TCP packet

                    ASSERT(TCPSocket::belongsToAnyTCPSocket(msg));

                    socket = libm->incomingtcpsocket(msg);
                    if(socket < 0)
                    {
                        // unknown socket (socket establishment?)

                        if(!strcmp(msg->name(), "ESTABLISHED"))
                        {
                            // create new socket
                            int csocket = libm->createTcpSocket(msg);

                            // find parent socket
                            socket = libm->findparentsocket(csocket);

                            // put in the list for accept
                            libm->enqueuConn(socket, csocket);
                        }
                        else
                        {
                            ASSERT(false);
                        }
                    }
                    else
                    {
                        // known socket (incoming data?)
                        TCPSocket *tcp = libm->gettcpsocket(socket);
                        tcp->processMessage(msg);
                    }
                }
                else if(ipControlInfo)
                {
                    // IP Packet

                    ASSERT(ipControlInfo->protocol() == IP_PROT_OSPF);

                    ASSERT(!strcmp(msg->name(), "data"));

                    socket = libm->incomingrawsocket(msg);

                    if(socket < 0)
                    {
                        ev << "no recipient (socket) for this message, discarding" << endl;
                        delete msg;
                        continue;
                    }

                    ASSERT(socket >= 0);

                    libm->enqueue(socket, msg);
                }
                else
                {
                    ASSERT(false);
                }

                if(FD_ISSET(socket, readfds))
                {
                    ev << "event on watched socket=" << socket << endl;
                    ++success;
                    break;
                }
                else
                {
                    ev << "socket=" << socket << " not watched, ignore" << endl;
                    continue;
                }

            }

        }

    }

    ASSERT(success);

    ev << "there are " << success << " sockets ready" << endl;

    for(int i = FD_MIN; i < nfds; i++)
    {
        //ev << "testing " << i << endl;

        if(FD_ISSET(i, readfds))
        {
            //ev << " watched ";

            if(!libm->getqueuetail(i) && libm->getaccepthead(i) == -1)
            {
                FD_CLR(i, readfds);
                //ev << " clear" << endl;
            }
            else
            {
                //ev << " active" << endl;
            }
        }
        else
        {
            //ev << " not watched" << endl;
        }

        // write is always ready

    }

    // exceptions not supported yet
    FD_ZERO(errorfds);

    return success;
}

int oppsim_close(int fildes)
{
    Daemon *libm = DAEMON;

    ev << "close: fildes=" << fildes << endl;

    if(libm->issocket(fildes))
    {
        ev << "closing pseudo socket" << endl;

        if(libm->isudpsocket(fildes))
        {
            UDPSocket *udp = libm->getudpsocket(fildes);
            udp->close();
            libm->closesocket(fildes);

            return 0;
        }

        // closing TCP/NETLINK not implemented
        ASSERT(false);
    }

    if(libm->isfile(fildes))
    {
        ev << "closing file descriptor" << endl;

        libm->closefile(fildes);

        return 0;
    }


    // invalid descriptor
    ASSERT(false);
}

void oppsim_perror(const char *s)
{
    ev << "perror: " << s << ": " << strerror(GlobalVars_errno()) << endl;

    ASSERT(false);
}

ssize_t oppsim_writev(int fildes, const struct iovec *iov, int iovcnt)
{
    ev << "writev: fildes=" << fildes << " iov=" << iov << " iovcnt=" << iovcnt << endl;

    Daemon *libm = DAEMON;

    ASSERT(libm->isfile(fildes));

    FILE *stream = libm->getstream(fildes);

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
    Daemon *libm = DAEMON;

    ev << "write: fildes=" << fildes << " buf=" << buf << " nbyte=" << nbyte << endl;

    if(libm->issocket(fildes))
    {
        if(libm->istcpsocket(fildes))
        {
            ev << "write into tcp socket" << endl;

            SocketMsg *msg = new SocketMsg("data");

            msg->setDataArraySize(nbyte);
            for(int i = 0; i < nbyte; i++)
                msg->setData(i, ((char*)buf)[i]);

            msg->setLength(nbyte * 8);

            TCPSocket *tcp = libm->gettcpsocket(fildes);
            tcp->send(msg);

            return nbyte;
        }

        // write to NETLINK/UDP shouldn't be used
        ASSERT(false);
    }

    if(libm->isfile(fildes))
    {
        ev << "regular file descriptor, use fwrite" << endl;

        return fwrite(buf, 1, nbyte, libm->getstream(fildes));
    }

    // writing into invalid descriptor
    ASSERT(false);
}

int oppsim_listen(int socket, int backlog)
{
    Daemon *libm = DAEMON;

    ev << "listen: socket=" << socket << " backlog=" << backlog << endl;

    libm->gettcpsocket(socket)->listen();

    return 0;
}


int oppsim_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    Daemon *libm = DAEMON;

    ev << "bind: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    ASSERT(address);

    if(address->sa_family == AF_INET)
    {
        const struct sockaddr_in *inaddr = (sockaddr_in*)address;

        int port = ntohs(inaddr->sin_port);
        int addr = ntohl(inaddr->sin_addr.s_addr);

        ev << "binding to address=" << IPAddress(addr) << " port=" << port << endl;

        if(libm->istcpsocket(socket))
        {
            libm->gettcpsocket(socket)->bind(IPAddress(addr), port);

            return 0;
        }

        if(libm->isudpsocket(socket))
        {
            libm->getudpsocket(socket)->bind(IPAddress(addr), port);

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
            ev << "autogenerating nl_pid" << endl;
        }

        libm->getnlsocket(socket)->bind(pid);

        ev << "binding to nl_pid=" << pid << endl;

        return 0;
    }

    ASSERT(false);
}

int oppsim_accept(int socket, struct sockaddr *address, socklen_t *address_len)
{
    Daemon *libm = DAEMON;

    ev << "accept: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    int csocket = libm->getaccepthead(socket, true);
    ASSERT(csocket != -1);

    if(address)
    {
        ASSERT(address_len);
        ASSERT(*address_len == sizeof(sockaddr_in));
        struct sockaddr_in *inaddr = (sockaddr_in*)address;

        TCPSocket *tcp = libm->gettcpsocket(csocket);
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
    Daemon *libm = DAEMON;

    ev << "read: fildes=" << fildes << " buf=" << buf << " nbyte=" << nbyte << endl;

    if(libm->issocket(fildes))
    {
        if(libm->istcpsocket(fildes))
        {
            ev << "reading from TCP socket" << endl;

            int bread = 0;

            char *str = (char*)buf;

            while(nbyte > 0)
            {
                cMessage *m = libm->getqueuetail(fildes, false);
                if(!m)
                    break;

                SocketMsg *msg = check_and_cast<SocketMsg*>(m);

                int size = msg->getDataArraySize();
                int min = size < nbyte? size: nbyte;

                for(int i = 0; i < min; i++)
                {
                    *(str++) = msg->getData(i);
                    ++bread;
                    --nbyte;
                }

                if(min == size)
                {
                    // remove consumed message from queue

                    libm->getqueuetail(fildes, true);
                }
                else
                {
                    // remove read characters from message

                    ASSERT(min < size);
                    ASSERT(nbyte == 0);

                    for(int i = min; i < size; i++)
                    {
                        msg->setData(i - min, msg->getData(i));
                    }
                    msg->setDataArraySize(size - min);

                }
            }

            ASSERT(bread > 0);

            return bread;
        }

        // read from NETLINK/UDP shouldn't be used
        ASSERT(false);
    }

    if(libm->isfile(fildes))
    {
        // currently not implemented, use fread on libm->getstream(fildes) if necessary
        ASSERT(false);
    }

    // reading from invalid descriptor
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
    Daemon *libm = DAEMON;

    ev << "getcwd: buf=" << (void*)buf << " size=" << size << endl;

    std::string cwd = libm->getcwd();

    const char *cwdstr = cwd.c_str();

    ASSERT(strlen(cwdstr) < size);

    strncpy(buf, cwdstr, size);

    ev << "cwd=" << buf << endl;

    return buf;
}

int oppsim_chdir(const char *path)
{
    ASSERT(false);
    return -1;
}

struct passwd *oppsim_getpwnam(const char *name)
{
    Daemon *libm = DAEMON;

    ev << "getpwnam: name=" << name << endl;

    return &libm->pwd_entry;
}

struct group *oppsim_getgrnam(const char *name)
{
    Daemon *libm = DAEMON;

    ev << "getgrnam: name=" << name << endl;

    return &libm->grp_entry;
}

int oppsim_setgroups(size_t size, const gid_t *list)
{
    ASSERT(false);
    return -1;
}

int oppsim_connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    Daemon *libm = DAEMON;

    ev << "connect: socket=" << socket << " address=" << address << " address_len=" << address_len << endl;

    struct sockaddr_in *inaddr = (sockaddr_in*)address;
    ASSERT(address_len == sizeof(*inaddr));
    IPAddress destAddr = IPAddress(ntohl(inaddr->sin_addr.s_addr));

    ev << "destAddress=" << destAddr << endl;

    TCPSocket *tcp = libm->gettcpsocket(socket);

    if(tcp->localAddress().isUnspecified())
    {
        // XXX FIXME this should be probably fixed in TCP layer

        IPAddress localAddr;

        RoutingTable *rt = check_and_cast<RoutingTable*>(libm->parentModule()->submodule("routingTable"));
        if(rt->localDeliver(destAddr))
        {
            localAddr = destAddr;
        }
        else
        {
            int outp = rt->outputPortNo(destAddr);
            ASSERT(outp >= 0);
            InterfaceTable *ift = check_and_cast<InterfaceTable*>(libm->parentModule()->submodule("interfaceTable"));
            localAddr = ift->interfaceByPortNo(outp)->ipv4()->inetAddress();
        }

        ev << "autoassigning local address=" << localAddr << endl;

        tcp->bind(localAddr, -1);
    }

    tcp->connect(destAddr, ntohs(inaddr->sin_port));

    ev << "connect to destAddr=" << destAddr << " port=" << ntohs(inaddr->sin_port) << endl;

    // XXX FIXME block until connection is established ???
    // INET seems to queue request arriving prematurely

    return 0;
}

int oppsim_getpeername(int socket, struct sockaddr *address, socklen_t *address_len)
{
    ASSERT(false);
    return -1;
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

int oppsim_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    Daemon *libm = DAEMON;

    ev << "sigaction: sig=" << sig << " act=" << act << " oact=" << oact << endl;

    struct sigaction *sptr = libm->sigactionimpl(sig);

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
