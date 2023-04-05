//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFcsInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetFcsInserter);

void EthernetFcsInserter::initialize(int stage)
{
    FcsInserterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        insertFcs = par("insertFcs");
        setFcs = par("setFcs");
    }
}

uint32_t EthernetFcsInserter::computeFcs(const Packet *packet, FcsMode fcsMode) const
{
    switch (fcsMode) {
        case FCS_DECLARED_CORRECT:
            return computeDeclaredCorrectFcs(packet);
        case FCS_DECLARED_INCORRECT:
            return computeDeclaredIncorrectFcs(packet);
        case FCS_COMPUTED:
            return computeComputedFcs(packet);
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
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

void EthernetFcsInserter::processPacket(Packet *packet)
{
    if (insertFcs) {
        const auto& header = makeShared<EthernetFcs>();
        packet->insertAtBack(header);
        auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
        packetProtocolTag->setProtocol(&Protocol::ethernetMac);
        packetProtocolTag->setFrontOffset(b(0));
        packetProtocolTag->setBackOffset(b(0));
    }

    if (setFcs) {
        auto header = packet->removeAtBack<EthernetFcs>(ETHER_FCS_BYTES);
        if (auto cutthroughTag = packet->findTag<CutthroughTag>()) {
            if (cutthroughTag->isGoodTrailer()) {
                auto entirePacket = new Packet();
                destreamChunkToPacket(entirePacket, packet->peekData());
                header->setFcsMode(fcsMode);
                header->setFcs(computeFcs(entirePacket, fcsMode));
                delete entirePacket;
            }
            else {
                switch (fcsMode) {
                    case FCS_DECLARED_CORRECT:
                    case FCS_DECLARED_INCORRECT:
                        header->setFcsMode(FCS_DECLARED_INCORRECT);
                        header->setFcs(computeDeclaredIncorrectFcs(packet));
                        break;
                    case FCS_COMPUTED:
                        header->setFcsMode(FCS_COMPUTED);
                        header->setFcs(0x0f0f0f0fL);
                        break;
                    default:
                        throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
                        break;
                }
            }
        }
        else {
            auto fcs = computeFcs(packet, fcsMode);
            header->setFcs(fcs);
            header->setFcsMode(fcsMode);
        }
        packet->insertAtBack(header);
    }
}

} // namespace inet
