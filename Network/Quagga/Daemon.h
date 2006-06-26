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

#define	FD_EXIST(a)		(a >= 0 && a < fd.size() && a < FD_SETSIZE)

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
    
		TCPSocket* getIfTcpSocket(int socket);
		TCPSocket* getTcpSocket(int socket);
		UDPSocket* getIfUdpSocket(int socket);
		UDPSocket* getUdpSocket(int socket);
		RawSocket* getIfRawSocket(int socket);
		RawSocket* getRawSocket(int socket);
		Netlink* getIfNetlinkSocket(int socket);
		Netlink* getNetlinkSocket(int socket);
		FILE* getIfStream(int fildes);
		FILE* getStream(int fildes);
        
        bool isBlocking(int fildes);

		int getEmptySlot();
    
        int createTcpSocket(cMessage *msg = NULL);
        int createUdpSocket();
        int createRawSocket(int protocol);
        int createNetlinkSocket();
        int createStream(const char *path, char *mode);
        
        void handleReceivedMessage(cMessage *msg);
        bool receiveAndHandleMessage(double timeout);
        
        bool hasQueuedConnections(int socket);
        int acceptTcpSocket(int socket);
        void enqueueConnection(int socket, int csocket);
        cMessage* getSocketMessage(int socket, bool remove=false);
        void enqueueSocketMessage(int socket, cMessage *msg);
        
        void closeSocket(int socket);
        void closeStream(int fildes);

        int findTcpSocket(cMessage *msg);
        int findRawSocket(int protocol);
		int findServerSocket(TCPConnectInfo *info);

        void setBlocked(bool b);

        virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
        virtual void socketEstablished(int connId, void *yourPtr);
        virtual void socketPeerClosed(int connId, void *yourPtr) { ASSERT(false); }
        virtual void socketClosed(int connId, void *yourPtr) { ASSERT(false); }
        virtual void socketFailure(int connId, void *yourPtr, int code) { ASSERT(false); }


        struct_sigaction* sigactionimpl(int signo);
        std::string getcwd();
        std::string getrootprefix();


    public:

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
            std::list<int> incomingQueue; // queued conns
            bool blocking;
        };

        std::vector<lib_descriptor_t> fd;
        
    private:        

        std::vector<struct_sigaction> sig;

        std::string cwd;
        std::string rootprefix;

        TCPSocketMap socketMap;

    public:

        bool blocked;
        
        int euid;
};

extern Daemon *current_module;

#define DAEMON          (check_and_cast<Daemon*>(simulation.runningModule()))

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif

