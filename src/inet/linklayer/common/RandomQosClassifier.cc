//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/common/RandomQosClassifier.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"

namespace inet {

Define_Module(RandomQosClassifier);

void RandomQosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(intrand(8));
    send(msg, "out");
}

void RandomQosClassifier::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (!strcmp("in", g->getName()))
        registerService(protocol, gate("out"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

void RandomQosClassifier::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("in", g->getName()))
        registerProtocol(protocol, gate("out"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

} // namespace inet

