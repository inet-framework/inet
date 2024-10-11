//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLC_H
#define __INET_IEEE8022LLC_H

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022Llc : public OperationalBase, public IPassivePacketSink
{
  protected:
    struct SocketDescriptor {
        int socketId = -1;
        int localSap = -1;
        int remoteSap = -1;

        SocketDescriptor(int socketId, int localSap, int remoteSap = -1)
            : socketId(socketId), localSap(localSap), remoteSap(remoteSap) {}
    };

    friend std::ostream& operator<<(std::ostream& o, const SocketDescriptor& t);

    PassivePacketSinkRef upperLayerSink;
    PassivePacketSinkRef lowerLayerSink;

    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

  protected:
    void clearSockets();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);
    virtual void processPacketFromHigherLayer(Packet *msg);
    virtual bool deliverCopyToSockets(Packet *packet);  // return true when delivered to any socket
    virtual bool hasUpperProtocol(const Protocol *protocol);
    virtual bool isDeliverableToUpperLayer(Packet *packet);
    virtual void processPacketFromMac(Packet *packet);
    virtual void processCommandFromHigherLayer(Request *request);

    // for lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

  public:
    virtual ~Ieee8022Llc();
    static const Protocol *getProtocol(const Ptr<const Ieee8022LlcHeader>& header);
};

} // namespace inet

#endif

