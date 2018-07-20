//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010-2011 Zoltan Bojthe
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

#ifndef __INET_TCP_H
#define __INET_TCP_H

#include <map>
#include <set>

#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

namespace tcp {

// Forward declarations:
class TcpConnection;
class TcpSendQueue;
class TcpReceiveQueue;

/**
 * Implements the Tcp protocol. This section describes the internal
 * architecture of the Tcp model.
 *
 * Usage and compliance with various RFCs are discussed in the corresponding
 * NED documentation for Tcp. Also, you may want to check the TcpSocket
 * class which makes it easier to use Tcp from applications.
 *
 * The Tcp protocol implementation is composed of several classes (discussion
 * follows below):
 *  - Tcp: the module class
 *  - TcpConnection: manages a connection
 *  - TcpSendQueue, TcpReceiveQueue: abstract base classes for various types
 *    of send and receive queues
 *  - TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue which implement
 *    queues with "virtual" bytes (byte counts only)
 *  - TcpAlgorithm: abstract base class for Tcp algorithms, and subclasses:
 *    DumbTcp, TcpBaseAlg, TcpTahoeRenoFamily, TcpTahoe, TcpReno, TcpNewReno.
 *
 * Tcp subclassed from cSimpleModule. It manages socketpair-to-connection
 * mapping, and dispatches segments and user commands to the appropriate
 * TcpConnection object.
 *
 * TcpConnection manages the connection, with the help of other objects.
 * TcpConnection itself implements the basic Tcp "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 * SYN, FIN, RST, ACKs, etc.
 *
 * TcpConnection internally relies on 3 objects. The first two are subclassed
 * from TcpSendQueue and TcpReceiveQueue. They manage the actual data stream,
 * so TcpConnection itself only works with sequence number variables.
 * This makes it possible to easily accomodate need for various types of
 * simulated data transfer: real byte stream, "virtual" bytes (byte counts
 * only), and sequence of cMessage objects (where every message object is
 * mapped to a Tcp sequence number range).
 *
 * Currently implemented send queue and receive queue classes are
 * TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue which implement
 * queues with "virtual" bytes (byte counts only).
 *
 * The third object is subclassed from TcpAlgorithm. Control over
 * retransmissions, congestion control and ACK sending are "outsourced"
 * from TcpConnection into TcpAlgorithm: delayed acks, slow start, fast rexmit,
 * etc. are all implemented in TcpAlgorithm subclasses. This simplifies the
 * design of TcpConnection and makes it a lot easier to implement new Tcp
 * variations such as NewReno, Vegas or LinuxTcp as TcpAlgorithm subclasses.
 *
 * Currently implemented TcpAlgorithm classes are TcpReno, TcpTahoe, TcpNewReno,
 * TcpNoCongestionControl and DumbTcp.
 *
 * The concrete TcpAlgorithm class to use can be chosen per connection (in OPEN)
 * or in a module parameter.
 */
class INET_API Tcp : public cSimpleModule, public ILifecycle
{
  public:
    static simsignal_t tcpConnectionAddedSignal;
    static simsignal_t tcpConnectionRemovedSignal;

    struct AppConnKey    // XXX this class is redundant since connId is already globally unique
    {
        int socketId;

        inline bool operator<(const AppConnKey& b) const
        {
            return socketId < b.socketId;
        }
    };
    struct SockPair
    {
        L3Address localAddr;
        L3Address remoteAddr;
        int localPort;    // -1: unspec
        int remotePort;    // -1: unspec

        inline bool operator<(const SockPair& b) const
        {
            if (remoteAddr != b.remoteAddr)
                return remoteAddr < b.remoteAddr;
            else if (localAddr != b.localAddr)
                return localAddr < b.localAddr;
            else if (remotePort != b.remotePort)
                return remotePort < b.remotePort;
            else
                return localPort < b.localPort;
        }
    };

  protected:
    typedef std::map<AppConnKey, TcpConnection *> TcpAppConnMap;
    typedef std::map<SockPair, TcpConnection *> TcpConnMap;
    TcpCrcInsertion crcInsertion;

    TcpAppConnMap tcpAppConnMap;
    TcpConnMap tcpConnMap;

    ushort lastEphemeralPort = static_cast<ushort>(-1);
    std::multiset<ushort> usedEphemeralPorts;

  protected:
    /** Factory method; may be overriden for customizing Tcp */
    virtual TcpConnection *createConnection(int socketId);

    // utility methods
    virtual TcpConnection *findConnForSegment(const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr);
    virtual TcpConnection *findConnForApp(int socketId);
    virtual void segmentArrivalWhileClosed(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address src, L3Address dest);
    virtual void removeConnection(TcpConnection *conn);
    virtual void refreshDisplay() const override;

  public:
    static bool testing;    // switches between tcpEV and testingEV
    static bool logverbose;    // if !testing, turns on more verbose logging

    bool recordStatistics = false;    // output vectors on/off
    bool isOperational = false;    // lifecycle: node is up/down

    bool useDataNotification = false;
    CrcMode crcMode = CRC_MODE_UNDEFINED;

  public:
    Tcp() {}
    virtual ~Tcp();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  public:
    /**
     * To be called from TcpConnection when a new connection gets created,
     * during processing of OPEN_ACTIVE or OPEN_PASSIVE.
     */
    virtual void addSockPair(TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

    /**
     * To be called from TcpConnection when socket pair (key for TcpConnMap) changes
     * (e.g. becomes fully qualified).
     */
    virtual void updateSockPair(TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

    /**
     * Update conn's socket pair, and register newConn (which'll keep LISTENing).
     * Also, conn will get a new socketId (and newConn will live on with its old socketId).
     */
    virtual void addForkedConnection(TcpConnection *conn, TcpConnection *newConn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

    /**
     * To be called from TcpConnection: reserves an ephemeral port for the connection.
     */
    virtual ushort getEphemeralPort();

    /**
     * To be called from TcpConnection: create a new send queue.
     */
    virtual TcpSendQueue *createSendQueue();

    /**
     * To be called from TcpConnection: create a new receive queue.
     */
    virtual TcpReceiveQueue *createReceiveQueue();

    // ILifeCycle:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    // called at shutdown/crash
    virtual void reset();

    bool checkCrc(const Ptr<const TcpHeader>& tcpHeader, Packet *pk);
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_H

