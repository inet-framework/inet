//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4SOCKETCOMMANDPROCESSOR_H
#define __INET_IPV4SOCKETCOMMANDPROCESSOR_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4SocketPacketProcessor.h"
#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4SocketTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

class INET_API Ipv4SocketCommandProcessor : public OperationalMixin<queueing::PacketFlowBase>, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<Ipv4SocketTable> socketTable;
    ModuleRef<Ipv4SocketPacketProcessor> socketPacketProcessorModule;
    cGate *socketOutGate = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void processPushCommand(Message *message, cGate *arrivalGate);
    virtual void processPacket(Packet *packet) override {}
    virtual void pushOrSendCommand(Message *command, cGate *outGate, Ipv4SocketPacketProcessor *consumer);

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    /**
     * ILifecycle methods
     */
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
};

} // namespace inet

#endif
