//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_QUIC_QUIC_H_
#define __INET_QUIC_QUIC_H_

#include <omnetpp.h>
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/transportlayer/contract/quic/IQuic.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "Connection.h"
#include "UdpSocket.h"
#include "AppSocket.h"
#include "inet/networklayer/contract/IRoutingTable.h"

using namespace omnetpp;

namespace inet {
namespace quic {

// Forward declarations:
class Connection;
class UdpSocket;

class Quic : public OperationalBase, public IQuic, public queueing::IPassivePacketSink, public IModuleInterfaceLookup
{
  public:
    virtual void setCallback(int socketId, IQuic::ICallback *callback) override;
    virtual void bind(int socketId, const L3Address& localAddr, uint16_t localPort) override;
    virtual void listen(int socketId) override;
    virtual void connect(int socketId, const L3Address& remoteAddr, uint16_t remotePort) override;
    virtual void accept(int socketId, int newSocketId) override;
    virtual void recv(int socketId, uint64_t streamId, int64_t expectedDataSize) override;
    virtual void close(int socketId) override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

    virtual ~Quic();
    virtual Connection *createConnection(UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort);
    virtual UdpSocket *createUdpSocket();
    virtual UdpSocket *findUdpSocket(L3Address addr, uint16_t port);
    virtual AppSocket *findOrCreateAppSocket(int socketId);
    virtual void addConnection(uint64_t connectionId, Connection *connection);
    virtual void removeConnectionId(uint64_t connectionId);
    IRoutingTable *getRoutingTable();

  protected:
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_TRANSPORT_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_TRANSPORT_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_TRANSPORT_LAYER; }

  private:
    std::map<uint64_t, Connection *> connectionIdConnectionMap;
    std::map<int, UdpSocket *> udpSocketIdUdpSocketMap;
    std::map<int, AppSocket *> socketIdAppSocketMap;
    uint64_t nextConnectionId = 0;

    void handleTimeout(cMessage *msg);
    void handleMessageFromApp(cMessage *msg);
    void handleMessageFromUdp(cMessage *msg);
    void addConnection(Connection *connection);
    void addUdpSocket(UdpSocket *udpSocket);
    Connection *findConnection(uint64_t srcConnectionId);
    Connection *findConnectionByDstConnectionId(uint64_t connectionId, Packet *pkt);
    UdpSocket *findUdpSocket(int socketId);
    uint64_t extractConnectionId(Packet *pkt, bool *readSourceConnectionId = nullptr);
    void destroySockets(AppSocket *appSocket);
    bool isUdpSocketInUse(UdpSocket *udpSocket);
};

} //namespace quic
} //namespace inet

#endif
