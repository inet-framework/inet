//
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __TCP_LWIP_CONNECTION_H
#define __TCP_LWIP_CONNECTION_H

#include <omnetpp.h>

#include "IPvXAddress.h"
#include "lwip/tcp.h"

// forward declarations:
class TCP_lwip;
class TCPConnectInfo;
class TcpLwipReceiveQueue;
class TcpLwipSendQueue;
class INetStack;
class INetStreamSocket;
class TCPStatusInfo;

/**
 *
 */
class INET_API TcpLwipConnection
{
    // prevent copy constructor:
    TcpLwipConnection(const TcpLwipConnection&);
  public:

    TcpLwipConnection(TCP_lwip &tcpLwipP, int connIdP, int gateIndexP, const char *sendQueueClassP, const char *recvQueueClassP);

    TcpLwipConnection(TcpLwipConnection &tcpLwipConnectionP, int connIdP, LwipTcpLayer::tcp_pcb *pcbP);

    ~TcpLwipConnection();

    void sendEstablishedMsg();

    static const char* indicationName(int code);

    void sendIndicationToApp(int code);

    void listen(IPvXAddress& localAddr, unsigned short localPort);

    void connect(IPvXAddress& localAddr, unsigned short localPort, IPvXAddress& remoteAddr, unsigned short remotePort);

    void close();

    void abort();

    void send(cPacket *msgP);

    void fillStatusInfo(TCPStatusInfo &statusInfo);

    int send_data(void *data, int len);

    void do_SEND();

    INetStreamSocket* getSocket();

  public:
    int connIdM;
    int appGateIndexM;

    INetStreamSocket *pLwipSocketM;

    bool sentEstablishedM;
    bool onCloseM;
    bool isListenerM;

    TCP_lwip &tcpLwipM;
    TcpLwipReceiveQueue * receiveQueueM;
    TcpLwipSendQueue * sendQueueM;
    LwipTcpLayer::tcp_pcb *pcbM;
};

#endif
