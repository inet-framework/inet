//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetCutthroughSource.h"

#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetCutthroughSource);

void EthernetCutthroughSource::initialize(int stage)
{
    PacketDestreamer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cutthroughSwitchingHeaderSize = b(par("cutthroughSwitchingHeaderSize"));
        networkInterface = getContainingNicModule(this);
        macForwardingTable.reference(this, "macTableModule", true);
        cutthroughTimer = new cMessage("CutthroughTimer");
    }
}

void EthernetCutthroughSource::handleMessage(cMessage *message)
{
    if (message == cutthroughTimer) {
        auto cutthroughPacket = streamedPacket->dup();
        b cutthroughPosition = getCutthroughSwitchingHeaderSize(cutthroughPacket);
        auto cutthroughData = cutthroughPacket->removeAt(cutthroughPosition, cutthroughPacket->getDataLength() - cutthroughPosition - ETHER_FCS_BYTES);
        cutthroughData->markImmutable();
        cutthroughBuffer = makeShared<StreamBufferChunk>(cutthroughData, simTime(), datarate);
        cutthroughPacket->insertAt(cutthroughBuffer, cutthroughPosition);
        cutthroughPacket->copyRegionTags(*streamedPacket, cutthroughPosition, cutthroughPosition, cutthroughData->getChunkLength());
        cutthroughPacket->addTag<CutthroughTag>()->setCutthroughPosition(cutthroughPosition);
        EV_INFO << "Sending cut-through packet" << EV_FIELD(packet, *cutthroughPacket) << EV_ENDL;
        pushOrSendPacket(cutthroughPacket, outputGate, consumer);
        cutthroughInProgress = true;
    }
    else
        PacketDestreamer::handleMessage(message);
}

b EthernetCutthroughSource::getCutthroughSwitchingHeaderSize(Packet *packet) const
{
    if (cutthroughSwitchingHeaderSize != b(0))
        return cutthroughSwitchingHeaderSize;
    else {
        EthernetCutthroughDissectorCallback callback;
        PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), callback);
        packetDissector.dissectPacket(packet);
        return callback.cutthroughSwitchingHeaderSize - ETHER_FCS_BYTES;
    }
}

bool EthernetCutthroughSource::EthernetCutthroughDissectorCallback::shouldDissectProtocolDataUnit(const Protocol *protocol)
{
    return protocol == &Protocol::ethernetPhy || protocol == &Protocol::ethernetMac ||
           protocol == &Protocol::ieee8021qCTag || protocol == &Protocol::ieee8021qSTag || protocol == &Protocol::ieee8021rTag;
}

void EthernetCutthroughSource::EthernetCutthroughDissectorCallback::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    if (protocol == &Protocol::ethernetPhy || protocol == &Protocol::ethernetMac ||
        protocol == &Protocol::ieee8021qCTag || protocol == &Protocol::ieee8021qSTag || protocol == &Protocol::ieee8021rTag)
    {
        cutthroughSwitchingHeaderSize += chunk->getChunkLength();
    }
    else
        payloadProtocol = protocol;
}

void EthernetCutthroughSource::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    PacketDestreamer::pushPacketStart(packet, gate, datarate);
    if (!cutthroughInProgress && isEligibleForCutthrough(packet)) {
        b cutthroughPosition = getCutthroughSwitchingHeaderSize(packet);
        simtime_t delay = s(cutthroughPosition / datarate).get();
        scheduleAt(simTime() + delay, cutthroughTimer);
        updateDisplayString();
    }
}

void EthernetCutthroughSource::pushPacketEnd(Packet *packet, cGate *gate)
{
    if (cutthroughInProgress) {
        Enter_Method("pushPacketEnd");
        take(packet);
        delete streamedPacket;
        streamedPacket = nullptr;
        cutthroughInProgress = false;
        b cutthroughPosition = getCutthroughSwitchingHeaderSize(packet);
        auto cutthroughData = packet->removeAt(cutthroughPosition, packet->getDataLength() - cutthroughPosition - ETHER_FCS_BYTES);
        cutthroughData->markImmutable();
        cutthroughBuffer->setStreamData(cutthroughData);
        cutthroughBuffer = nullptr;
        numProcessedPackets++;
        processedTotalLength += packet->getTotalLength();
        delete packet;
        updateDisplayString();
    }
    else
        PacketDestreamer::pushPacketEnd(packet, gate);
}

bool EthernetCutthroughSource::isEligibleForCutthrough(Packet *packet) const
{
    EthernetCutthroughDissectorCallback callback;
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), callback);
    packetDissector.dissectPacket(packet);
    return callback.payloadProtocol != &Protocol::gptp;
}

} // namespace inet

