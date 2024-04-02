//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetLayer.h"

namespace inet {

Define_Module(EthernetLayer);

void EthernetLayer::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ethernetMac, gate("upperLayerIn"), gate("upperLayerOut"));
        registerProtocol(Protocol::ethernetMac, gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

} // namespace inet

