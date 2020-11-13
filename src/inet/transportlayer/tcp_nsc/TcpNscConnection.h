//
// Copyright (C) 2006 Sam Jansen
// Copyright (C) 2006 OpenSim Ltd.
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_TCPNSCCONNECTION_H
#define __INET_TCPNSCCONNECTION_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"

// forward declarations:
struct INetStack;
struct INetStreamSocket;

namespace inet {

class TcpConnectInfo;

namespace tcp {

class TcpNsc;
class TcpNscReceiveQueue;
class TcpNscSendQueue;

/**
 * Encapsulates a Network Simulation Cradle (NSC) instance.
 */
class INET_API TcpNscConnection
{
  public:
    class SockAddr
    {
      public:
        SockAddr() : ipAddrM(), portM(-1) {}
        L3Address ipAddrM;
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
    TcpNscConnection();
    ~TcpNscConnection();

    void listen(INetStack& stackP, SockPair& inetSockPairP, SockPair& nscSockPairP);
    void connect(INetStack& stackP, SockPair& inetSockPairP, SockPair& nscSockPairP);
    void close();
    void abort();
    void send(Packet *msgP);
    void do_SEND();

  public:
    int connIdM = -1;
    int forkedConnId = -1;    // identifies forked connection within the app (listener socket ID)
    SockPair inetSockPairM;
    SockPair nscSockPairM;
    INetStreamSocket *pNscSocketM = nullptr;

    bool sentEstablishedM = false;
    bool onCloseM = false;
    bool disconnectCalledM = false;
    bool isListenerM = false;

    // TCP Windows Size
    int tcpWinSizeM;

    TcpNsc *tcpNscM = nullptr;
    TcpNscReceiveQueue *receiveQueueM = nullptr;
    TcpNscSendQueue *sendQueueM = nullptr;
};

} // namespace tcp

} // namespace inet

#endif

