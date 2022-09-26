//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACRELAYUNITBASE_H
#define __INET_MACRELAYUNITBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API MacRelayUnitBase : public LayeredProtocolBase, public StringFormat::IDirectiveResolver
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IMacForwardingTable> macForwardingTable;

    long numProcessedFrames = 0;
    long numDroppedFrames = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void updateDisplayString() const;
    virtual std::string resolveDirective(char directive) const override;

    virtual bool isForwardingInterface(NetworkInterface *networkInterface) const { return !networkInterface->isLoopback() && networkInterface->isBroadcast(); }
    virtual void broadcastPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *incomingInterface);
    virtual void sendPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *outgoingInterface);
    virtual void updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId);

    //@{ for lifecycle:
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
    virtual bool isUpperMessage(cMessage *message) const override { return message->arrivedOn("upperLayerIn"); }
    virtual bool isLowerMessage(cMessage *message) const override { return message->arrivedOn("lowerLayerIn"); }

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    //@}
};

} // namespace inet

#endif

