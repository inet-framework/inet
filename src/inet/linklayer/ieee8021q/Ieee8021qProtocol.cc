//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qProtocol.h"

namespace inet {

Define_Module(Ieee8021qProtocol);

void Ieee8021qProtocol::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        const Protocol *qtagProtocol;
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qSTag;
        else if (!strcmp("c", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qCTag;
        else
            throw cRuntimeError("Unknown tag type");
        registerService(*qtagProtocol, gate("upperLayerIn"), gate("upperLayerOut"));
        registerProtocol(*qtagProtocol, gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

} // namespace inet

