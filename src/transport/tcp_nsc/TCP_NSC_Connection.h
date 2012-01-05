//
// Copyright (C) 2006 Sam Jansen, Andras Varga, 2009 Zoltan Bojthe
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


#ifndef __TCP_NSC_CONNECTION_H
#define __TCP_NSC_CONNECTION_H

#ifndef HAVE_NSC
#error Please install NSC or disable 'TCP_NSC' feature
#endif


#include "INETDefs.h"

#include "IPvXAddress.h"

// forward declarations:
class TCPConnectInfo;
class TCP_NSC;
class TCP_NSC_ReceiveQueue;
class TCP_NSC_SendQueue;
class INetStack;
class INetStreamSocket;

/**
 * Encapsulates a Network Simulation Cradle (NSC) instance.
 */
class INET_API TCP_NSC_Connection
{
  public:
    class SockAddr
    {
      public:
        SockAddr() : ipAddrM(), portM(-1) {}
        IPvXAddress ipAddrM;
        unsigned short portM;

        inline bool operator<(const SockAddr& b) const
        {
            if (ipAddrM == b.ipAddrM)
                return portM < b.portM;
            return ipAddrM < b.ipAddrM;
        }

        inline bool operator==(const SockAddr& b) const
        {
            return (ipAddrM == b.ipAddrM) && (portM == b.portM);
        }
    };

    class SockPair
    {
      public:
        SockAddr remoteM;
        SockAddr localM;

        inline bool operator<(const SockPair& b) const
        {
            if (remoteM == b.remoteM)
                return localM < b.localM;
            return remoteM < b.remoteM;
        }

        inline bool operator==(const SockPair& b) const
        {
            return (remoteM == b.remoteM) && (localM == b.localM);
        }
    };

  public:
    TCP_NSC_Connection();
    cMessage* createEstablishedMsg();
    void listen(INetStack &stackP, SockPair &inetSockPairP, SockPair &nscSockPairP);
    void connect(INetStack &stackP, SockPair &inetSockPairP, SockPair &nscSockPairP);
    void close();
    void abort();
    void send(cPacket *msgP);
    void do_SEND();

  public:
    int connIdM;
    int appGateIndexM;
    SockPair inetSockPairM;
    SockPair nscSockPairM;
    INetStreamSocket *pNscSocketM;

    bool sentEstablishedM;
    bool onCloseM;
    bool isListenerM;

    // TCP Windows Size
    int tcpWinSizeM;

    TCP_NSC *tcpNscM;
    TCP_NSC_ReceiveQueue * receiveQueueM;
    TCP_NSC_SendQueue * sendQueueM;
};

#endif
