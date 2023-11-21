//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/PacketEventTag.h"

namespace inet {

void insertPacketEvent(const cModule *module, Packet *packet, int kind, simtime_t bitDuration, simtime_t packetDuration, PacketEvent *packetEvent)
{
    auto simulation = module->getSimulation();
    packet->mapAllRegionTagsForUpdate<PacketEventTag>(b(0), packet->getTotalLength(), [&] (b offset, b length, const Ptr<PacketEventTag>& eventTag) {
        auto packetEventCopy = packetEvent->dup();
        packetEventCopy->setKind(kind);
        packetEventCopy->setModulePath(module->getFullPath().c_str());
        packetEventCopy->setEventNumber(simulation->getEventNumber());
        packetEventCopy->setSimulationTime(simulation->getSimTime());
        packetEventCopy->setBitDuration(bitDuration);
        packetEventCopy->setPacketDuration(packetDuration);
        packetEventCopy->setPacketLength(packet->getDataLength());
        eventTag->appendPacketEvents(packetEventCopy);
    });
    delete packetEvent;
}

} // namespace inet

