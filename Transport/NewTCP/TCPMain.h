//
// Copyright (C) 2004 Andras Varga
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

#ifndef __TCPMAIN_H
#define __TCPMAIN_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <omnetpp.h>
#include <map>
#include "IPAddress.h"


class TCPConnection;
class TCPSegment;

/**
 * Implements the TCP protocol.
 *
 * This class only manages connections (TCPConnection) and dispatches
 * messages and events to them. Everything of interest takes place
 * in TCPConnection.
 */
class TCPMain : public cSimpleModule
{
  protected:
    struct AppConnKey
    {
        int appGateIndex;
        int connId;

        inline bool operator<(const AppConnKey& b) const
        {
            if (appGateIndex!=b.appGateIndex)
                return appGateIndex<b.appGateIndex;
            else
                return connId<b.connId;
        }

    };
    struct SockPair
    {
        int localAddr;
        int remoteAddr;
        short localPort;
        short remotePort;

        inline bool operator<(const SockPair& b) const
        {
            if (remoteAddr!=b.remoteAddr)
                return remoteAddr<b.remoteAddr;
            else if (localAddr!=b.localAddr)
                return localAddr<b.localAddr;
            else if (remotePort!=b.remotePort)
                return remotePort<b.remotePort;
            else
                return localPort<b.localPort;
        }
    };

    typedef std::map<AppConnKey,TCPConnection*> TcpAppConnMap;
    typedef std::map<SockPair,TCPConnection*> TcpConnMap;

    TcpAppConnMap tcpAppConnMap;
    TcpConnMap tcpConnMap;

    short nextEphemeralPort;

    TCPConnection *findConnForSegment(TCPSegment *tcpseg, IPAddress srcAddr, IPAddress destAddr);
    TCPConnection *findConnForApp(int appGateIndex, int connId);
    void removeConnection(TCPConnection *conn);

  public:
    Module_Class_Members(TCPMain, cSimpleModule, 0);
    virtual ~TCPMain();
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    /**
     * To be called from TCPConnection when socket pair (key for TcpConnMap) changes
     * (e.g. becomes fully qualified).
     */
    void updateSockPair(TCPConnection *conn, IPAddress localAddr, IPAddress remoteAddr, int localPort, int remotePort);

    /**
     * To be called from TCPConnection: reserves an ephemeral port for the connection.
     */
    short getEphemeralPort();

};

#endif


