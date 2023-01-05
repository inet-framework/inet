//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetCutthroughBarrier.h"

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetCutthroughBarrier);

void EthernetCutthroughBarrier::processPacket(Packet *packet, simtime_t sendingTime)
{
    if (auto cutthroughTag = packet->findTag<CutthroughTag>()) {
        EV_DEBUG << "Detected cut-through packet" << EV_FIELD(packet) << EV_ENDL;
        b cutthroughPosition = cutthroughTag->getCutthroughPosition();
        auto cutthroughBuffer = packet->removeAt<StreamBufferChunk>(cutthroughTag->getCutthroughPosition(), packet->getTotalLength() - cutthroughPosition - ETHER_FCS_BYTES);
        auto cutthroughData = cutthroughBuffer->getStreamData();
        EV_DEBUG << "Replacing cut-through stream buffer with cut-through data" << EV_FIELD(cutthroughBuffer) << EV_FIELD(cutthroughData) << EV_ENDL;
        packet->insertAt(cutthroughData, cutthroughPosition);
        packet->removeTag<CutthroughTag>();
        EV_DEBUG << "Complete cut-through packet" << EV_FIELD(packet) << EV_ENDL;
    }
    PacketDelayerBase::processPacket(packet, sendingTime);
}

clocktime_t EthernetCutthroughBarrier::computeDelay(Packet *packet) const
{
    if (auto cutthroughTag = packet->findTag<CutthroughTag>()) {
         EV_DEBUG << "Detected cut-through packet" << EV_FIELD(packet) << EV_ENDL;
         b cutthroughPosition = cutthroughTag->getCutthroughPosition();
         auto cutthroughBuffer = packet->peekAt<StreamBufferChunk>(cutthroughTag->getCutthroughPosition(), packet->getTotalLength() - cutthroughPosition - ETHER_FCS_BYTES);
         return SIMTIME_AS_CLOCKTIME(cutthroughBuffer->getStartTime() + s(cutthroughBuffer->getChunkLength() / cutthroughBuffer->getDatarate()).get());
    }
    else
        return 0;
}


} // namespace inet

