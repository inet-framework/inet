//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/base/TransportProtocolBase.h"

namespace inet {

bool TransportProtocolBase::isUpperMessage(cMessage *msg) const
{
    return msg->arrivedOn("appIn");
}

bool TransportProtocolBase::isLowerMessage(cMessage *msg) const
{
    return msg->arrivedOn("ipIn");
}

bool TransportProtocolBase::isInitializeStage(int stage) const
{
    return stage == INITSTAGE_TRANSPORT_LAYER;
}

bool TransportProtocolBase::isModuleStartStage(int stage) const
{
    return stage == ModuleStartOperation::STAGE_TRANSPORT_LAYER;
}

bool TransportProtocolBase::isModuleStopStage(int stage) const
{
    return stage == ModuleStopOperation::STAGE_TRANSPORT_LAYER;
}

} // namespace inet

