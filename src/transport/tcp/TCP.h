//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPMAIN_H
#define __INET_TCPMAIN_H

#include <map>
#include <set>
#include <omnetpp.h>
#include "IPvXAddress.h"


class TCPConnection;
class TCPSegment;

// macro for normal ev<< logging (Note: deliberately no parens in macro def)
#define tcpEV (ev.disable_tracing||TCP::testing)?ev:ev

// macro for more verbose ev<< logging (Note: deliberately no parens in macro def)
#define tcpEV2 (ev.disable_tracing||TCP::testing||!TCP::logverbose)?ev:ev

// testingEV writes log that automated test cases can check (*.test files)
#define testingEV (ev.disable_tracing||!TCP::testing)?ev:ev





/**
 * Implements the TCP protocol. This section describes the internal
 * architecture of the TCP model.
 *
 * Usage and compliance with various RFCs are discussed in the corresponding
 * NED documentation for TCP. Also, you may want to check the TCPSocket
 * class which makes it easier to use TCP from applications.
 *
 * The TCP protocol implementation is composed of several classes (discussion
 * follows below):
 *  - TCP: the module class
 *  - TCPConnection: manages a connection
 *  - TCPSendQueue, TCPReceiveQueue: abstract base classes for various types
 *    of send and receive queues
 *  - TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue which implement
 *    queues with "virtual" bytes (byte counts only)
 *  - TCPAlgorithm: abstract base class for TCP algorithms, and subclasses:
 *    DumbTCP, TCPBaseAlg, TCPTahoeRenoFamily, TCPTahoe, TCPReno, TCPNewReno.
 *
 * TCP subclassed from cSimpleModule. It manages socketpair-to-connection
 * mapping, and dispatches segments and user commands to the appropriate
 * TCPConnection object.
 *
 * TCPConnection manages the connection, with the help of other objects.
 * TCPConnection itself implements the basic TCP "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 * SYN, FIN, RST, ACKs, etc.
 *
 * TCPConnection internally relies on 3 objects. The first two are subclassed
 * from TCPSendQueue and TCPReceiveQueue. They manage the actual data stream,
 * so TCPConnection itself only works with sequence number variables.
 * This makes it possible to easily accomodate need for various types of
 * simulated data transfer: real byte stream, "virtual" bytes (byte counts
 * only), and sequence of cMessage objects (where every message object is
 * mapped to a TCP sequence number range).
 *
 * Currently implemented send queue and receive queue classes are
 * TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue which implement
 * queues with "virtual" bytes (byte counts only).
 *
 * The third object is subclassed from TCPAlgorithm. Control over
 * retransmissions, congestion control and ACK sending are "outsourced"
 * from TCPConnection into TCPAlgorithm: delayed acks, slow start, fast rexmit,
 * etc. are all implemented in TCPAlgorithm subclasses. This simplifies the
 * design of TCPConnection and makes it a lot easier to implement new TCP
 * variations such as NewReno, Vegas or LinuxTCP as TCPAlgorithm subclasses.
 *
 * Currently implemented TCPAlgorithm classes are TCPReno, TCPTahoe, TCPNewReno,
 * TCPNoCongestionControl and DumbTCP.
 *
 * The concrete TCPAlgorithm class to use can be chosen per connection (in OPEN)
 * or in a module parameter.
 */
class INET_API TCP : public cSimpleModule
{
  public:
    struct AppConnKey  // XXX this class is redundant since connId is already globally unique
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
        IPvXAddress localAddr;
        IPvXAddress remoteAddr;
        int localPort;   // -1: unspec
        int remotePort;  // -1: unspec

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

  protected:
    typedef std::map<AppConnKey,TCPConnection*> TcpAppConnMap;
    typedef std::map<SockPair,TCPConnection*> TcpConnMap;

    TcpAppConnMap tcpAppConnMap;
    TcpConnMap tcpConnMap;

    ushort lastEphemeralPort;
    std::multiset<ushort> usedEphemeralPorts;

  protected:
    /** Factory method; may be overriden for customizing TCP */
    virtual TCPConnection *createConnection(int appGateIndex, int connId);

    // utility methods
    virtual TCPConnection *findConnForSegment(TCPSegment *tcpseg, IPvXAddress srcAddr, IPvXAddress destAddr);
    virtual TCPConnection *findConnForApp(int appGateIndex, int connId);
    virtual void segmentArrivalWhileClosed(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);
    virtual void removeConnection(TCPConnection *conn);
    virtual void updateDisplayString();

  public:
    static bool testing;    // switches between tcpEV and testingEV
    static bool logverbose; // if !testing, turns on more verbose logging

    bool recordStatistics;  // output vectors on/off

  public:
    TCP() {}
    virtual ~TCP();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    /**
     * To be called from TCPConnection when a new connection gets created,
     * during processing of OPEN_ACTIVE or OPEN_PASSIVE.
     */
    virtual void addSockPair(TCPConnection *conn, IPvXAddress localAddr, IPvXAddress remoteAddr, int localPort, int remotePort);

    /**
     * To be called from TCPConnection when socket pair (key for TcpConnMap) changes
     * (e.g. becomes fully qualified).
     */
    virtual void updateSockPair(TCPConnection *conn, IPvXAddress localAddr, IPvXAddress remoteAddr, int localPort, int remotePort);

    /**
     * Update conn's socket pair, and register newConn (which'll keep LISTENing).
     * Also, conn will get a new connId (and newConn will live on with its old connId).
     */
    virtual void addForkedConnection(TCPConnection *conn, TCPConnection *newConn, IPvXAddress localAddr, IPvXAddress remoteAddr, int localPort, int remotePort);

    /**
     * To be called from TCPConnection: reserves an ephemeral port for the connection.
     */
    virtual ushort getEphemeralPort();
};

#endif


