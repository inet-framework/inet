//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFcsChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetFcsChecker);

void EthernetFcsChecker::initialize(int stage)
{
    FcsCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isCheckFcs = par("checkFcs");
        popFcs = par("popFcs");
    }
}

bool EthernetFcsChecker::checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const
{
    switch (fcsMode) {
        case FCS_DECLARED_CORRECT:
            return checkDeclaredCorrectFcs(packet, fcs);
        case FCS_DECLARED_INCORRECT:
            return checkDeclaredIncorrectFcs(packet, fcs);
        case FCS_COMPUTED:
            return checkComputedFcs(packet, fcs);
        default:
            throw cRuntimeError("Unknown FCS mode");
    }
}

// TODO duplicated code (see EthernetFcsInserter), move to a utility file
static void destreamChunkToPacket(Packet *outPacket, const Ptr<const Chunk> data)
{
    switch (data->getChunkType()) {
        case Chunk::CT_STREAM:
            destreamChunkToPacket(outPacket, dynamicPtrCast<const StreamBufferChunk>(data)->getStreamData());
            break;
        case Chunk::CT_SEQUENCE:
            for (const auto& chunk : dynamicPtrCast<const SequenceChunk>(data)->getChunks())
                destreamChunkToPacket(outPacket, chunk);
            break;
        case Chunk::CT_SLICE:
        {
            auto slicePacket = new Packet();
            auto sliceChunk = dynamicPtrCast<const SliceChunk>(data);
            destreamChunkToPacket(slicePacket, sliceChunk->getChunk());
            outPacket->insertAtBack(slicePacket->peekDataAt(sliceChunk->getOffset(), sliceChunk->getLength()));
            delete slicePacket;
        }
        break;
        default:
            outPacket->insertAtBack(data);
            break;
    }
}

void EthernetFcsChecker::processPacket(Packet *packet)
{
    if (isCheckFcs) {
        if (auto cutthroughTag = packet->findTagForUpdate<CutthroughTag>()) {
            // make a packet copy with entire data for FCS calculation
            auto entirePacket = new Packet();
            destreamChunkToPacket(entirePacket, packet->peekData());
            const auto& trailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);
            bool isGoodFcs = checkFcs(entirePacket, trailer->getFcsMode(), trailer->getFcs());
            cutthroughTag->setTrailerChunk(trailer);
            cutthroughTag->setIsGoodTrailer(isGoodFcs);
            delete entirePacket;
        }
    }

    if (popFcs) {
        const auto& trailer = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
        auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() + trailer->getChunkLength());
    }
}

bool EthernetFcsChecker::matchesPacket(const Packet *packet) const
{
    if (!isCheckFcs || packet->hasTag<CutthroughTag>())
        return true;
    const auto& trailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    auto fcsMode = trailer->getFcsMode();
    auto fcs = trailer->getFcs();
    return checkFcs(packet, fcsMode, fcs);
}

void EthernetFcsChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

cGate *EthernetFcsChecker::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace inet

