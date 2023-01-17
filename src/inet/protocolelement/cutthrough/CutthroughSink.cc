//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/cutthrough/CutthroughSink.h"

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(CutthroughSink);

void CutthroughSink::endStreaming()
{
    if (auto cutthroughTag = streamedPacket->findTag<CutthroughTag>()) {
        auto cutthroughBuffer = streamedPacket->removeAt<StreamBufferChunk>(cutthroughTag->getCutthroughPosition());
        streamedPacket->insertAtBack(cutthroughBuffer->getStreamData());
        streamedPacket->removeTag<CutthroughTag>();
    }
    PacketStreamer::endStreaming();
}

} // namespace inet

