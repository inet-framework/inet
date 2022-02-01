//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PPP_H
#define __INET_PPP_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

/**
 * PPP implementation.
 */
class INET_API Ppp : public MacProtocolBase, public queueing::IActivePacketSink
{
  protected:
    const char *displayStringTextFormat = nullptr;
    bool sendRawBytes = false;
    cGate *physOutGate = nullptr;
    cChannel *datarateChannel = nullptr; // nullptr if we're not connected

    cMessage *endTransmissionEvent = nullptr;

    // saved current transmission
    Packet *curTxPacket = nullptr;

    std::string oldConnColor;

    // statistics
    long numSent = 0;
    long numRcvdOK = 0;
    long numDroppedBitErr = 0;
    long numDroppedIfaceDown = 0;

    static simsignal_t transmissionStateChangedSignal;
    static simsignal_t rxPkOkSignal;

  protected:
    virtual void startTransmitting();
    virtual void encapsulate(Packet *msg);
    virtual void decapsulate(Packet *packet);
    virtual void refreshDisplay() const override;
    virtual void refreshOutGateConnection(bool connected);

    // cListener function
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // MacBase functions
    virtual void configureNetworkInterface() override;

  public:
    virtual ~Ppp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;

    virtual void processUpperPacket();

  public:
    // IActivePacketSink:
    virtual queueing::IPassivePacketSource *getProvider(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif

