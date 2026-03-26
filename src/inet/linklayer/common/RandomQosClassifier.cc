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

void RandomQosClassifier::initialize()
{
    outSink.reference(gate("out"), true);
}

void RandomQosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(intrand(8));
    outSink.pushPacket(packet);
}

void RandomQosClassifier::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

} // namespace inet

