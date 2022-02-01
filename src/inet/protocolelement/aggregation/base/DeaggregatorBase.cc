//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/aggregation/base/DeaggregatorBase.h"

namespace inet {

void DeaggregatorBase::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        deleteSelf = par("deleteSelf");
}

void DeaggregatorBase::pushPacket(Packet *aggregatedPacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(aggregatedPacket);
    auto subpackets = deaggregatePacket(aggregatedPacket);
    for (auto subpacket : subpackets) {
        EV_INFO << "Deaggregating packet" << EV_FIELD(subpacket) << EV_FIELD(packet, *aggregatedPacket) << EV_ENDL;
        pushOrSendPacket(subpacket, outputGate, consumer);
    }
    processedTotalLength += aggregatedPacket->getDataLength();
    numProcessedPackets++;
    updateDisplayString();
    delete aggregatedPacket;
    if (deleteSelf)
        deleteModule();
}

} // namespace inet

