//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKPROTOCOLBASE_H
#define __INET_NETWORKPROTOCOLBASE_H

#include <map>
#include <set>

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3Protocol.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API NetworkProtocolBase : public LayeredProtocolBase, public IL3Protocol, public IPassivePacketSink
{
  protected:
    struct SocketDescriptor {
        int socketId = -1;
        int protocolId = -1;
        L3Address localAddress;
        L3Address remoteAddress;
        IL3Protocol::ICallback *callback = nullptr;

        SocketDescriptor(int socketId, int protocolId, L3Address localAddress)
            : socketId(socketId), protocolId(protocolId), localAddress(localAddress) {}
    };

    ModuleRefByPar<IInterfaceTable> interfaceTable;
    PassivePacketSinkRef transportSink;
    PassivePacketSinkRef queueSink;
    // working vars
    std::set<const Protocol *> upperProtocols;
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

  protected:
    NetworkProtocolBase();
    virtual ~NetworkProtocolBase() { for (auto entry : socketIdToSocketDescriptor) delete entry.second; }

    virtual void initialize(int stage) override;

    virtual void sendUp(cMessage *message);
    virtual void sendDown(cMessage *message, int interfaceId = -1);

    virtual bool isUpperMessage(cMessage *message) const override;
    virtual bool isLowerMessage(cMessage *message) const override;

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }

    virtual void handleUpperCommand(cMessage *msg) override;

    virtual const Protocol& getProtocol() const = 0;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual void setCallback(int socketId, ICallback *callback) override;
    virtual void bind(int socketId, const Protocol *protocol, const L3Address& localAddress) override;
    virtual void connect(int socketId, const L3Address& remoteAddress) override;
    virtual void close(int socketId) override;
    virtual void destroy(int socketId) override;
};

} // namespace inet

#endif

