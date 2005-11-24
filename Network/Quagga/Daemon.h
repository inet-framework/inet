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
#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <omnetpp.h>
#include <vector>

#include "TCPSocket.h"
#include "UDPSocket.h"
#include "RawSocket.h"
#include "Netlink.h"
#include "TCPSocketMap.h"

#include "zebra_env.h"

class Daemon : public cSimpleModule, public TCPSocket::CallbackInterface
{
    public:
        Module_Class_Members(Daemon, cSimpleModule, 32768);
        virtual void activity();


    public:
        struct passwd pwd_entry;
        struct group grp_entry;

        struct GlobalVars *varp;

    public:

        bool isfile(int fildes);
        bool issocket(int fildes);

        bool istcpsocket(int socket);
        bool isudpsocket(int socket);
        bool israwsocket(int socket);
        bool isnlsocket(int socket);

        int findemptydesc();

        TCPSocket* gettcpsocket(int socket);
        UDPSocket* getudpsocket(int socket);
        RawSocket* getrawsocket(int socket);
        Netlink* getnlsocket(int socket);
        FILE* getstream(int fildes);


        int createTcpSocket(cMessage *msg = NULL);
        int createRawSocket(int protocol);
        int createUdpSocket();
        int createNetlinkSocket();
        int createFile(const char *path, char *mode);

        void closesocket(int socket);
        void closefile(int fildes);

        int incomingtcpsocket(cMessage *msg);
        int incomingrawsocket(cMessage *msg);

        void setblocked(bool b);

        virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
        virtual void socketEstablished(int connId, void *yourPtr);
        virtual void socketPeerClosed(int connId, void *yourPtr) { ASSERT(false); }
        virtual void socketClosed(int connId, void *yourPtr) { ASSERT(false); }
        virtual void socketFailure(int connId, void *yourPtr, int code) { ASSERT(false); }


        struct sigaction* sigactionimpl(int signo);
        std::string getcwd();
        std::string getrootprefix();
        cMessage* getqueuetail(int socket, bool remove=false);
        int getaccepthead(int socket, bool remove=false);
        void enqueue(int socket, cMessage *msg);
        void enqueuConn(int socket, int csocket);

        int findparentsocket(int socket);

    private:

        enum fdtype
        {
            FD_TCP,
            FD_NETLINK,
            FD_UDP,
            FD_RAW,
            FD_FILE,
            FD_EMPTY
        };

        struct lib_descriptor_t
        {
            fdtype type;
            TCPSocket *tcp;
            UDPSocket *udp;
            RawSocket *raw;
            Netlink *netlink;
            FILE *stream;
            cQueue queue; // arrived messages
            std::list<int> serv; // queued conns
        };

        std::vector<lib_descriptor_t> fd;

        std::vector<struct sigaction> sig;

        std::string cwd;
        std::string rootprefix;

        TCPSocketMap socketMap;

        bool blocked;

    public:

        int euid;
};

extern Daemon *current_module;

#define DAEMON          (check_and_cast<Daemon*>(simulation.runningModule()))

// socket=16 is stored as fd[0], socket=17 as fd[1], etc.
#define FD_MIN          16
#define FD_SUB(a)       (a - FD_MIN)    // socket to fd-index
#define FD_ADD(a)       (a + FD_MIN)    // fd-index to socket

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif

