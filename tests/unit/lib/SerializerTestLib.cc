//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "SerializerTestLib.h"

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/packet/recorder/PcapReader.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

// Deserialize (dissect) the packet into typed chunks, re-serialize the result,
// and compare it byte-by-byte with the original content.
static bool roundTripOneRecord(Packet *packet)
{
    // reference bytes: the frame exactly as it enters the round-trip (raw wire
    // bytes plus any FCS the caller appended)
    const auto& originalBytes = packet->peekDataAsBytes();

    // deserialize: dissect the raw bytes into their most specific typed chunks
    // and reassemble them into a single content chunk (uses PacketProtocolTag)
    PacketDissector::ChunkBuilder builder;
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), builder);
    packetDissector.dissectPacket(packet);
    const auto& content = builder.getContent();

    // re-serialize
    Packet rebuilt("roundtrip");
    rebuilt.insertAtBack(content);
    const auto& rebuiltBytes = rebuilt.peekAllAsBytes();

    if (originalBytes->getChunkLength() != rebuiltBytes->getChunkLength()) {
        EV << "  Length differs: original " << originalBytes->getChunkLength()
           << " vs rebuilt " << rebuiltBytes->getChunkLength() << "\n";
        return false;
    }
    const auto& a = originalBytes->getBytes();
    const auto& b = rebuiltBytes->getBytes();
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            EV << "  Bytes differ at offset " << i << "\n"
               << "    original: " << originalBytes->str() << "\n"
               << "    rebuilt:  " << rebuiltBytes->str() << "\n";
            return false;
        }
    }
    return true;
}

bool testPcapSerialization(const char *filename, bool hasFcs)
{
    EV << "=== Testing file " << filename << "\n";
    PcapReader reader;
    // pass a null name format: naming would dissect every packet (needlessly, and it
    // would choke on Ethernet frames whose FCS we only append below for the round-trip)
    reader.openPcap(filename, nullptr);
    bool allGood = true;
    int i = 0;
    while (true) {
        auto pair = reader.readPacket();
        Packet *packet = pair.second;
        if (packet == nullptr)
            break;
        i++;
        EV << "  === Frame " << i << "\n";

        // skip truncated records (only the first incl_len bytes were captured)
        const auto& rawData = packet->peekData(Chunk::PF_ALLOW_ALL);
        if (rawData->isIncomplete()) {
            EV << "  Skipped truncated frame " << i << "\n";
            delete packet;
            continue;
        }

        // real captures usually omit the FCS, but the Ethernet dissector expects a
        // 4-byte FCS trailer -- append a computed one, matching the input on both sides
        auto protocolTag = packet->findTag<PacketProtocolTag>();
        const Protocol *protocol = protocolTag != nullptr ? protocolTag->getProtocol() : nullptr;
        if (!hasFcs && protocol == &Protocol::ethernetMac) {
            const auto& data = packet->peekDataAsBytes();
            auto fcsChunk = makeShared<EthernetFcs>();
            fcsChunk->setFcs(ethernetFcs(data->getBytes()));
            fcsChunk->setFcsMode(FCS_COMPUTED);
            packet->insertAtBack(fcsChunk);
        }

        if (roundTripOneRecord(packet))
            EV << "  Frame " << i << " is the same\n";
        else {
            EV << "  Frame " << i << " differs\n";
            allGood = false;
        }
        delete packet;
    }
    reader.closePcap();
    if (allGood)
        EV << "All frames are the same in file " << filename << "\n";
    else
        EV << "Some frames differ in file " << filename << "\n";
    return allGood;
}

} // namespace inet
