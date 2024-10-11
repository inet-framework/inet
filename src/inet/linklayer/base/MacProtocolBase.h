//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACPROTOCOLBASE_H
#define __INET_MACPROTOCOLBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API MacProtocolBase : public LayeredProtocolBase, public cListener, public IPassivePacketSink
{
  protected:
    /** @brief Gate ids */
    //@{
    int upperLayerInGateId = -1;
    int upperLayerOutGateId = -1;
    int lowerLayerInGateId = -1;
    int lowerLayerOutGateId = -1;
    //@}

    opp_component_ptr<NetworkInterface> networkInterface;

    opp_component_ptr<cModule> hostModule;
    PassivePacketSinkRef lowerLayerSink;
    PassivePacketSinkRef upperLayerSink;

    /** Currently transmitted frame if any */
    Packet *currentTxFrame = nullptr;

    /** Messages received from upper layer and to be transmitted later */
    opp_component_ptr<queueing::IPacketQueue> txQueue;

  protected:
    MacProtocolBase();
    virtual ~MacProtocolBase();

    virtual void initialize(int stage) override;

    virtual void registerInterface();
    virtual void configureNetworkInterface() = 0;

    virtual MacAddress parseMacAddressParameter(const char *addrstr);

    virtual void deleteCurrentTxFrame();
    virtual void dropCurrentTxFrame(PacketDropDetails& details);

    virtual void handleMessageWhenDown(cMessage *msg) override;

    virtual void sendUp(cMessage *message);
    virtual void sendDown(cMessage *message);

    virtual bool isUpperMessage(cMessage *message) const override;
    virtual bool isLowerMessage(cMessage *message) const override;

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }

    /**
     * should clear queue and emit signal "packetDropped" with entire packets
     */
    virtual void flushQueue(PacketDropDetails& details);

    /**
     * should clear queue silently
     */
    virtual void clearQueue();

    using cListener::receiveSignal;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    queueing::IPacketQueue *getQueue(cGate *gate) const;

    virtual bool canDequeuePacket() const;
    virtual Packet *dequeuePacket();

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return canPushSomePacket(gate); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

