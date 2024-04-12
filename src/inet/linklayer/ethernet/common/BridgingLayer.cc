//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/BridgingLayer.h"

namespace inet {

Define_Module(BridgingLayer);

void BridgingLayer::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        registerAnyService(gate("upperLayerIn"), gate("upperLayerOut"));
        registerAnyProtocol(gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

cGate *BridgingLayer::getRegistrationForwardingGate(cGate *g)
{
    auto fullName = g->getFullName();
    if (!strcmp("upperLayerOut", fullName))
        return gate("lowerLayerIn");
    else if (!strcmp("upperLayerIn", fullName))
        return gate("lowerLayerOut");
    else if (!strcmp("lowerLayerOut", fullName))
        return gate("upperLayerIn");
    else if (!strcmp("lowerLayerIn", fullName))
        return gate("upperLayerOut");
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace inet

