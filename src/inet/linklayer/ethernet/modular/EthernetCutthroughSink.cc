//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetCutthroughSink.h"

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetCutthroughSink);

void EthernetCutthroughSink::endStreaming()
{
    if (auto cutthroughTag = streamedPacket->findTag<CutthroughTag>()) {
        EV_DEBUG << "Detected cut-through packet" << EV_FIELD(streamedPacket) << EV_ENDL;
        b cutthroughPosition = cutthroughTag->getCutthroughPosition();
        auto cutthroughBuffer = streamedPacket->removeAt<StreamBufferChunk>(cutthroughTag->getCutthroughPosition(), streamedPacket->getDataLength() - cutthroughPosition - B(4));
        auto cutthroughData = cutthroughBuffer->getStreamData();
        EV_DEBUG << "Replacing cut-through stream buffer with cut-through data" << EV_FIELD(cutthroughBuffer) << EV_FIELD(cutthroughData) << EV_ENDL;
        streamedPacket->insertAt(cutthroughData, cutthroughPosition);
        streamedPacket->removeTag<CutthroughTag>();
        EV_DEBUG << "Complete cut-through packet" << EV_FIELD(streamedPacket) << EV_ENDL;
    }
    PacketStreamer::endStreaming();
}

} // namespace inet

