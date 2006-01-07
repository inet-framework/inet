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
#include "Daemon.h"

#include "SocketMsg.h"

#include "IPv4InterfaceData.h"

#include "IPControlInfo_m.h"

#define QUAGGA_UID  100
#define QUAGGA_GID  100

Define_Module(Daemon);

extern "C" {

int zebra_main_entry (int argc, char **argv);
int ripd_main_entry (int argc, char **argv);
int ospfd_main_entry (int argc, char **argv);

};

#include "oppsim_kernel.h"

//static int zebra_num = 0;
//static int ospf_num = 0;

void Daemon::activity()
{
    // cached pointer used by DAEMON macro
    current_module = this;
    // global variable structure pointer
    varp = GlobalVars_createActiveSet();
    ASSERT(varp);
    __activeVars = varp;
    // initialize global variables
    GlobalVars_initializeActiveSet_lib();

    EV << "Quagga daemon starting" << endl;
    EV << " with variable context " << varp << endl;

    blocked = false;
    cwd = "/";
    rootprefix = par("rootfs").stringValue();
    euid = 0; // daemon starts as root

    // stdin, stdout, stderr
    lib_descriptor_t fd_std;
    fd_std.type = FD_FILE;
    fd_std.stream = stdin;
    fd.push_back(fd_std);
    fd_std.stream = stdout;
    fd.push_back(fd_std);
    fd_std.stream = stderr;
    fd.push_back(fd_std);

    pwd_entry.pw_uid = QUAGGA_UID;
    grp_entry.gr_gid = QUAGGA_GID;

    for(int i = 0; i < 32; i++)
    {
        struct_sigaction sa;
        sa.sa_handler = SIG_DFL;
        //sigemptyset(&sa.sa_mask); XXX FIXME memset to zero?
        sa.sa_flags = 0;
        sig.push_back(sa);
    }

    const char *server = par("server").stringValue();

    char **cmdline;
    cmdline = (char**)malloc(2);
    cmdline[0] = const_cast<char *>(server);
    cmdline[1] = NULL;

    if(!strcmp(server, "zebra"))
    {

        // FIXME debug only
        //++zebra_num;
        //if(zebra_num > 2)
        //  wait(uniform(0, 0.001) + 10);
        //else


        // randomize start
        wait(uniform(0, 0.001));
        current_module = this;
        __activeVars = varp;
        GlobalVars_initializeActiveSet_zebra();

        EV << "ready for zebra_main_entry()" << endl;

        zebra_main_entry(1, cmdline);
    }
    else if(!strcmp(server, "ripd"))
    {
        // randomize start
        wait(uniform(0.001, 0.002));
        current_module = this;
        __activeVars = varp;
        GlobalVars_initializeActiveSet_ripd();

        EV << "ready for ripd_main_entry()" << endl;

        ripd_main_entry(1, cmdline);
    }
    else if(!strcmp(server, "ospfd"))
    {

        // FIXME debug only
        //++ospf_num;
        //if(ospf_num > 2)
        //  wait(uniform(0, 0.001) + 10);
        //else


        // randomize start
        wait(uniform(0.002, 0.003));
        current_module = this;
        __activeVars = varp;
        GlobalVars_initializeActiveSet_ospfd();

        EV << "ready for ospfd_main_entry()" << endl;

        ospfd_main_entry(1, cmdline);
    }
    else
    {
        EV << "daemon " << server << " not recognized" << endl;

        ASSERT(false);
    }
}

TCPSocket* Daemon::getIfTcpSocket(int socket)
{
    ASSERT(FD_EXIST(socket));
    return (fd[socket].type == FD_TCP)? fd[socket].tcp: NULL;
}

TCPSocket* Daemon::getTcpSocket(int socket)
{
    TCPSocket *tcp = getIfTcpSocket(socket);
    ASSERT(tcp);
    return tcp;
}

UDPSocket* Daemon::getIfUdpSocket(int socket)
{
    ASSERT(FD_EXIST(socket));
    return (fd[socket].type == FD_UDP)? fd[socket].udp: NULL;
}

UDPSocket* Daemon::getUdpSocket(int socket)
{
    UDPSocket *udp = getIfUdpSocket(socket);
    ASSERT(udp);
    return udp;
}

RawSocket* Daemon::getIfRawSocket(int socket)
{
    ASSERT(FD_EXIST(socket));
    return (fd[socket].type == FD_RAW)? fd[socket].raw: NULL;
}

RawSocket* Daemon::getRawSocket(int socket)
{
    RawSocket *raw = getIfRawSocket(socket);
    ASSERT(raw);
    return raw;
}

Netlink* Daemon::getIfNetlinkSocket(int socket)
{
    ASSERT(FD_EXIST(socket));
    return (fd[socket].type == FD_NETLINK)? fd[socket].netlink: NULL;
}

Netlink* Daemon::getNetlinkSocket(int socket)
{
    Netlink *nl = getIfNetlinkSocket(socket);
    ASSERT(nl);
    return nl;
}

FILE* Daemon::getIfStream(int fildes)
{
    ASSERT(FD_EXIST(fildes));
    return (fd[fildes].type == FD_FILE)? fd[fildes].stream: NULL;
}

FILE* Daemon::getStream(int fildes)
{
    FILE *stream = getIfStream(fildes);
    ASSERT(stream);
    return stream;
}

int Daemon::getEmptySlot()
{
    // return existing if any
    for(unsigned int i = 0; i < fd.size(); i++)
    {
        if(fd[i].type != FD_EMPTY)
            continue;

        return i;
    }

    // make sure there is enough room
    ASSERT(fd.size() < FD_SETSIZE);

    // create new slot
    lib_descriptor_t newItem;
    newItem.type = FD_EMPTY;
    fd.push_back(newItem);
    return fd.size()-1;
}

int Daemon::createTcpSocket(cMessage *msg)
{
    int socket = getEmptySlot();

    ASSERT(FD_EXIST(socket));
    ASSERT(fd[socket].type == FD_EMPTY);

    TCPSocket *tcp = msg? new TCPSocket(msg): new TCPSocket();
    cGate *g = gate("to_tcp_interface");
    ASSERT(g);
    tcp->setOutputGate(g);
    tcp->setCallbackObject(this, (void*)socket);

    lib_descriptor_t newItem;
    newItem.type = FD_TCP;
    newItem.tcp = tcp;
    fd[socket] = newItem;

    EV << "created new TCP socket=" << socket << endl;

    socketMap.addSocket(tcp);

    return socket;
}

int Daemon::createUdpSocket()
{
    int socket = getEmptySlot();

    ASSERT(FD_EXIST(socket));
    ASSERT(fd[socket].type == FD_EMPTY);

    UDPSocket *udp = new UDPSocket();
    cGate *g = gate("to_udp_interface");
    ASSERT(g);
    udp->setOutputGate(g);
    udp->setUserId(socket);

    lib_descriptor_t newItem;
    newItem.type = FD_UDP;
    newItem.udp = udp;
    fd[socket] = newItem;

    EV << "created new UDP socket=" << socket << endl;

    return socket;
}

int Daemon::createRawSocket(int protocol)
{
    int socket = getEmptySlot();

    ASSERT(FD_EXIST(socket));
    ASSERT(fd[socket].type == FD_EMPTY);

    RawSocket *raw = new RawSocket(socket, protocol);
    cGate *g = gate("to_ip_interface");
    ASSERT(g);
    raw->setOutputGate(g);

    lib_descriptor_t newItem;
    newItem.type = FD_RAW;
    newItem.raw = raw;
    fd[socket] = newItem;

    EV << "created new UDP socket=" << socket << endl;

    return socket;
}

int Daemon::createNetlinkSocket()
{
    int socket = getEmptySlot();

    ASSERT(FD_EXIST(socket));
    ASSERT(fd[socket].type == FD_EMPTY);

    Netlink *nl = new Netlink();

    lib_descriptor_t newItem;
    newItem.type = FD_NETLINK;
    newItem.netlink = nl;
    fd[socket] = newItem;

    EV << "created new Netlink socket=" << socket << endl;

    return socket;
}

int Daemon::createStream(const char *path, char *mode)
{
    int fdesc = getEmptySlot();

    ASSERT(FD_EXIST(fdesc));
    ASSERT(fd[fdesc].type == FD_EMPTY);

    lib_descriptor_t newItem;
    newItem.type = FD_FILE;
    newItem.stream = fopen(path, mode);
    fd[fdesc] = newItem;

    ASSERT(newItem.stream);

    EV << "created new file descriptor=" << fdesc << " for file=" << path << endl;

    return fdesc;
}

int Daemon::acceptTcpSocket(int socket, bool remove)
{
    ASSERT(FD_EXIST(socket));
    ASSERT(fd[socket].type == FD_TCP);

    if(fd[socket].incomingQueue.empty())
        return -1;

    int ret = fd[socket].incomingQueue.front();

    if(remove)
        fd[socket].incomingQueue.pop_front();

    return ret;
}

void Daemon::enqueueConnection(int socket, int csocket)
{
    ASSERT(getIfTcpSocket(socket));
    ASSERT(getIfTcpSocket(csocket));
    ASSERT(socket < csocket);
    fd[socket].incomingQueue.push_back(csocket);
}

cMessage* Daemon::getSocketMessage(int socket, bool remove)
{
    ASSERT(FD_EXIST(socket));

    if(fd[socket].queue.empty())
        return NULL;

    cMessage *msg = (cMessage*)fd[socket].queue.tail();

    if(remove)
        fd[socket].queue.remove(msg);

    return msg;
}

void Daemon::enqueueSocketMessage(int socket, cMessage *msg)
{
    ASSERT(FD_EXIST(socket));

    fd[socket].queue.insert(msg);
}

void Daemon::closeSocket(int socket)
{
    ASSERT(FD_EXIST(socket));

    if(fd[socket].type == FD_UDP)
    {
        ASSERT(fd[socket].udp);
        fd[socket].udp->close();
        delete fd[socket].udp;
        fd[socket].udp = NULL;
        fd[socket].type = FD_EMPTY;
        return;
    }

    // closing tcp, raw or netlink socket currently not implemented/used
    ASSERT(false);
}

void Daemon::closeStream(int fildes)
{
    ASSERT(FD_EXIST(fildes));
    ASSERT(fd[fildes].type == FD_FILE);
    ASSERT(fd[fildes].stream);

    fclose(fd[fildes].stream);
    fd[fildes].stream = NULL;
    fd[fildes].type = FD_EMPTY;
}

int Daemon::findTcpSocket(cMessage *msg)
{
    ASSERT(TCPSocket::belongsToAnyTCPSocket(msg));

    TCPSocket *socket = socketMap.findSocketFor(msg);
    if(!socket)
    {
        EV << "connection not found" << endl;
        return -1;
    }

    ASSERT(socket);

    for(int i = 0; i < fd.size(); i++)
    {
        if(fd[i].type != FD_TCP)
            continue;

        if(fd[i].tcp != socket)
            continue;

        return i;
    }

    ASSERT(false);
}

int Daemon::findRawSocket(int protocol)
{
    for(int i = 0; i < fd.size(); i++)
    {
        if(fd[i].type != FD_RAW)
            continue;

        if(fd[i].raw->getProtocol() != protocol)
            continue;

        return i;
    }

    return -1;
}

int Daemon::findServerSocket(TCPConnectInfo *info)
{
    // XXX FIXME this may not work in many cases
    // some support from underlying tcp layer needed

    int port = info->localPort();
    IPAddress addr = info->localAddr().get4();

    for(unsigned int i = 0; i < fd.size(); i++)
    {
        if(fd[i].type != FD_TCP)
            continue;

        if(fd[i].tcp->localPort() != port)
            continue;

        if(!fd[i].tcp->localAddress().equals(addr))
            continue;

        return i;
    }

    ASSERT(false);
}

std::string Daemon::getcwd()
{
    return cwd;
}

std::string Daemon::getrootprefix()
{
    return rootprefix;
}

struct_sigaction * Daemon::sigactionimpl(int signo)
{
    ASSERT(signo >= 0);
    ASSERT(signo < sig.size());
    return &sig[signo];
}

void Daemon::setblocked(bool b)
{
    ASSERT(blocked != b);
    blocked = b;
}

void Daemon::socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent)
{
    int socket = (long)yourPtr;

    EV << "data arrived on socket=" << socket << endl;

    enqueueSocketMessage(socket, msg);
}

void Daemon::socketEstablished(int connId, void *yourPtr)
{
    EV << "socket=" << (long)yourPtr << " established" << endl;
}
