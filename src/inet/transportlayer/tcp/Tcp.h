//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCP_H
#define __INET_TCP_H

#include <map>
#include <set>

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/transportlayer/base/TransportProtocolBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

using namespace inet::queueing;

// Forward declarations:
class TcpConnection;
class TcpSendQueue;
class TcpReceiveQueue;

/**
 * Implements the TCP protocol. Usage and compliance with various RFCs are
 * discussed in the corresponding NED documentation.
 *
 * **Communication with clients**
 *
 * This section describes the interface between the Tcp module and its clients
 * (applications or higher-layer protocols). However, note that implementors
 * of client modules do not necessarily need to know the details described herein.
 * Instead, they can use the TcpSocket utility class which hides the details
 * of talking to Tcp under an easy-to-use class interface.

 * For communication between client applications and TCP, the TcpCommandCode
 * and TcpStatusInd enums are used as message kinds, and TcpCommand
 * and its subclasses are used as control info.
 *
 * To open a connection from a client app, send a cMessage to TCP with
 * TCP_C_OPEN_ACTIVE as message kind and a TcpOpenCommand object filled in
 * and attached to it as control info. (The peer TCP will have to be LISTENing;
 * the server app can achieve this with a similar cMessage but TCP_C_OPEN_PASSIVE
 * message kind.) With passive open, there's a possibility to cause the connection
 * "fork" on an incoming connection, leaving the original connection LISTENing
 * on the port (see the fork field in TcpOpenCommand). Note that TcpOpenCommand
 * allows tcpAlgorithmClass to be specified, which will override the setting
 * in the TCP module. Also note that TcpOpenCommand allows the client to
 * choose between "autoread" and "explicit-read" modes.
 *
 * The client can send data by assigning the TCP_C_SEND message kind to the data
 * packet and sending it to TCP. Received data will be forwarded by TCP to the
 * client as messages with the TCP_I_DATA message kind. This happens
 * automatically in "autoread" mode. In "explicit-read" mode, the client must
 * issue READ requests to receive data. READ requests are cMessages with the
 * TCP_C_READ message kind and a TcpReadCommand control info attached.
 *
 * To close, the client sends a cMessage to TCP with the TCP_C_CLOSE message kind
 * and TcpCommand control info.
 *
 * Tcp sends notifications to the application whenever there's a significant
 * change in the state of the connection: established, remote TCP closed,
 * closed, timed out, connection refused, connection reset, etc. These
 * notifications are also cMessages with message kind TCP_I_xxx
 * (TCP_I_ESTABLISHED, etc.) and TcpCommand as control info.
 *
 * One TCP module can serve several application modules, and several
 * connections per application. When talking to applications, a
 * connection is identified by the socketId that is assigned by the application in
 * the OPEN call.
 *
 * **Sockets**
 *
 * The TcpSocket C++ class is provided to simplify managing TCP connections
 * from applications. TcpSocket handles the job of assembling and sending
 * command messages (OPEN, CLOSE, etc) to TCP, and it also simplifies
 * the task of dealing with packets and notification messages coming from Tcp.
 *
 *
 * **Communication with the IP layer**
 *
 * The TCP model relies on sending and receiving L3AddressReq/L3AddressInd tags
 * attached to TCP segment packets.
 *
 *
 * **Architecture**
 *
 * This section describes the internal architecture of the TCP model.
 * The implementation is composed of several classes (discussion follows below):
 *  - Tcp: the main module class
 *  - TcpConnection: module class that manages a connection
 *  - TcpSendQueue, TcpReceiveQueue: send and receive queues
 *  - TcpAlgorithm: abstract base class for TCP algorithms: DumbTcp, TcpTahoe,
 *    TcpReno, TcpNewReno, etc.
 *
 * Tcp is subclassed from cSimpleModule. It manages socketpair-to-connection
 * mapping, and dispatches segments and user commands to the appropriate
 * TcpConnection object.
 *
 * TcpConnection manages the connection, with the help of other objects.
 * TcpConnection itself implements the basic Tcp "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 * SYN, FIN, RST, ACKs, etc.
 *
 * TcpConnection internally relies on 3 objects: send queue, receive queue,
 * and TCP algorithm. The first two are TcpSendQueue and TcpReceiveQueue.
 * They manage the actual data stream, so TcpConnection need to only concern
 * itself with sequence numbers.

 * The TCP algorithm class is subclassed from TcpAlgorithm. Control over
 * retransmissions, congestion control and ACK sending are "outsourced"
 * from TcpConnection into TcpAlgorithm: delayed acks, slow start, fast rexmit,
 * etc. are all implemented in TcpAlgorithm subclasses. This simplifies the
 * design of TcpConnection and makes it a lot easier to implement new Tcp
 * variations such as NewReno, Vegas or LinuxTcp as TcpAlgorithm subclasses.
 *
 * The concrete TcpAlgorithm class to use can be chosen per connection (in OPEN)
 * or in a module parameter.
 */
class INET_API Tcp : public TransportProtocolBase, public IPassivePacketSink
{
  public:
    static simsignal_t tcpConnectionAddedSignal;
    static simsignal_t tcpConnectionRemovedSignal;

    enum PortRange {
        EPHEMERAL_PORTRANGE_START = 1024,
        EPHEMERAL_PORTRANGE_END   = 5000
    };

    struct SockPair {
        L3Address localAddr;
        L3Address remoteAddr;
        int localPort; // -1: unspec
        int remotePort; // -1: unspec

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
    typedef std::map<int /*socketId*/, TcpConnection *> TcpAppConnMap;
    typedef std::map<SockPair, TcpConnection *> TcpConnMap;
    PassivePacketSinkRef appSink;
    PassivePacketSinkRef ipSink;
    TcpAppConnMap tcpAppConnMap;
    TcpConnMap tcpConnMap;

    ushort lastEphemeralPort = static_cast<ushort>(-1);
    std::multiset<ushort> usedEphemeralPorts;

  protected:
    /** Factory method; may be overriden for customizing Tcp */
    virtual TcpConnection *createConnection(int socketId);

    // utility methods
    virtual TcpConnection *findConnForSegment(const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr);
    virtual TcpConnection *findConnForApp(int socketId);
    virtual void segmentArrivalWhileClosed(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest);
    virtual void refreshDisplay() const override;

  public:
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int msl;

  public:
    Tcp() {}
    virtual ~Tcp();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    virtual void handleSelfMessage(cMessage *message) override;
    virtual void handleUpperCommand(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

  public:
    /**
     * To be called from TcpConnection when a new connection gets created,
     * during processing of OPEN_ACTIVE or OPEN_PASSIVE.
     */
    virtual void addSockPair(TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort);

    virtual void removeConnection(TcpConnection *conn);
    virtual void sendFromConn(cMessage *msg, const char *gatename, int gateindex = -1);

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
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // called at shutdown/crash
    virtual void reset();

    bool checkCrc(Packet *pk);
    int getMsl() { return msl; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace tcp
} // namespace inet

#endif

