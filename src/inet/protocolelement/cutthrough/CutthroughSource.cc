//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/cutthrough/CutthroughSource.h"

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(CutthroughSource);

CutthroughSource::~CutthroughSource()
{
    cancelAndDelete(cutthroughTimer);
}

void CutthroughSource::initialize(int stage)
{
    PacketDestreamer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cutthroughPosition = b(par("cutthroughPosition"));
        cutthroughTimer = new cMessage("CutthroughTimer");
    }
}

void CutthroughSource::handleMessage(cMessage *message)
{
    if (message == cutthroughTimer) {
        auto cutthroughPacket = streamedPacket->dup();
        auto cutthroughData = cutthroughPacket->removeDataAt(cutthroughPosition);
        cutthroughData->markImmutable();
        auto cutthroughBuffer = makeShared<StreamBufferChunk>(cutthroughData, simTime(), datarate);
        cutthroughPacket->insertAtBack(cutthroughBuffer);
        cutthroughPacket->addTag<CutthroughTag>()->setCutthroughPosition(cutthroughPosition);
        EV_INFO << "Sending cut-through packet" << EV_FIELD(packet, *cutthroughPacket) << EV_ENDL;
        pushOrSendPacket(cutthroughPacket, outputGate, consumer);
    }
    else
        PacketDestreamer::handleMessage(message);
}

void CutthroughSource::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    PacketDestreamer::pushPacketStart(packet, gate, datarate);
    scheduleAfter(s(cutthroughPosition / datarate).get(), cutthroughTimer);
}

void CutthroughSource::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    delete streamedPacket;
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    delete packet;
    updateDisplayString();
}

} // namespace inet

