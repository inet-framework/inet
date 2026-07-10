//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ldp/LdpClassifier.h"

#include "inet/networklayer/ldp/Ldp.h"

namespace inet {

Define_Module(LdpClassifier);

void LdpClassifier::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
        ldp.reference(this, "ldpModule", true);
}

void LdpClassifier::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

// IIngressClassifier implementation (method invoked by MPLS)

bool LdpClassifier::lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId)
{
    return ldp->classifyPacket(packet, outLabel, outInterfaceId);
}

} // namespace inet
