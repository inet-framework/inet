//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALLAYERBASE_H
#define __INET_PHYSICALLAYERBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IPhysicalLayer.h"
#include "inet/physicallayer/wireless/common/signal/WirelessSignal.h"

namespace inet {

namespace physicallayer {

class INET_API PhysicalLayerBase : public LayeredProtocolBase, public IPhysicalLayer
{
  protected:
    int upperLayerInGateId = -1;
    int upperLayerOutGateId = -1;
    int radioInGateId = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void handleSignal(WirelessSignal *signal);
    virtual void handleLowerMessage(cMessage *message) override;

    virtual void sendUp(cMessage *message);

    virtual bool isUpperMessage(cMessage *message) const override;
    virtual bool isLowerMessage(cMessage *message) const override;

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_PHYSICAL_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_PHYSICAL_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_PHYSICAL_LAYER; }
};

} // namespace physicallayer

} // namespace inet

#endif

