//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "SerializerTestLib.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/packet/recorder/PcapReader.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

// accumulated coverage state (WP2), spanning all files of one run
static std::set<std::string> usedSerializers;
static long rawLeafRegionCount = 0;
static std::vector<std::string> topLevelRawLines;

namespace {

// Like PacketDissector::ChunkBuilder (reassembles the dissected content), but also
// notices raw (BytesChunk/BitsChunk) leaves -- regions the dissector could not parse.
class CoverageBuilder : public PacketDissector::ICallback
{
    PacketDissector::ChunkBuilder builder;
    bool firstChunk = true;

  public:
    bool topLevelRaw = false; // the very first visited chunk was raw => nothing was parsed
    long rawChunkCount = 0;

    const Ptr<const Chunk> getContent() { return builder.getContent(); }

    virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
    virtual void startProtocolDataUnit(const Protocol *protocol) override {}
    virtual void endProtocolDataUnit(const Protocol *protocol) override {}
    virtual void markIncorrect() override {}

    virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override
    {
        const Chunk *c = chunk.get();
        bool isRaw = dynamic_cast<const BytesChunk *>(c) != nullptr || dynamic_cast<const BitsChunk *>(c) != nullptr;
        if (firstChunk) {
            topLevelRaw = isRaw;
            firstChunk = false;
        }
        if (isRaw)
            rawChunkCount++;
        builder.visitChunk(chunk, protocol);
    }
};

} // anonymous namespace

// Deserialize (dissect) the packet into typed chunks, re-serialize the result,
// and compare it byte-by-byte with the original content. Also record which
// regions stayed raw (unparsed) for the coverage report.
static bool roundTripOneRecord(Packet *packet, const char *filename, int frame)
{
    // reference bytes: the frame exactly as it enters the round-trip (raw wire
    // bytes plus any FCS the caller appended)
    const auto& originalBytes = packet->peekDataAsBytes();

    // deserialize: dissect the raw bytes into their most specific typed chunks
    // and reassemble them into a single content chunk (uses PacketProtocolTag)
    CoverageBuilder builder;
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), builder);
    packetDissector.dissectPacket(packet);
    const auto& content = builder.getContent();

    // note unparsed (raw-bytes) regions
    if (builder.topLevelRaw) {
        std::stringstream ss;
        ss << "TOPLEVEL-RAWBYTES " << filename << " frame " << frame
           << ": outermost protocol not parsed";
        topLevelRawLines.push_back(ss.str());
    }
    rawLeafRegionCount += builder.topLevelRaw ? builder.rawChunkCount - 1 : builder.rawChunkCount;

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

        if (roundTripOneRecord(packet, filename, i))
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

void beginSerializerCoverage()
{
    usedSerializers.clear();
    rawLeafRegionCount = 0;
    topLevelRawLines.clear();
    ChunkSerializerRegistry::getInstance().setUsedSerializerRecorder(&usedSerializers);
}

void reportSerializerCoverage()
{
    auto& registry = ChunkSerializerRegistry::getInstance();
    registry.setUsedSerializerRecorder(nullptr); // stop recording

    const auto& registered = registry.getRegisteredTypeNames();
    std::vector<std::string> untested;
    for (const auto& name : registered) // std::set -> sorted
        if (usedSerializers.find(name) == usedSerializers.end())
            untested.push_back(name);

    EV << "\n=== Serializer coverage (this run) ===\n";
    EV << "Registered serializers: " << registered.size() << "\n";
    EV << "Exercised: " << usedSerializers.size() << "\n";
    for (const auto& name : usedSerializers) // std::set -> sorted
        EV << "  USED " << name << "\n";
    EV << "Untested (registered but never invoked): " << untested.size() << "\n";
    for (const auto& name : untested)
        EV << "  UNTESTED " << name << "\n";

    EV << "\n=== Unparsed (raw-bytes) regions ===\n";
    EV << "Raw-bytes leaf regions: " << rawLeafRegionCount << " (expected: opaque payloads)\n";
    for (const auto& line : topLevelRawLines)
        EV << "  " << line << "\n";
    if (topLevelRawLines.empty())
        EV << "  (none)\n";
}

} // namespace inet
