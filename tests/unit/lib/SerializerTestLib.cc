//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "SerializerTestLib.h"

#include <cstdio>
#include <exception>
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

// Escape non-printable bytes so diagnostic text (e.g. an exception message that
// embeds a raw protocol byte) stays valid UTF-8 -- opp_test reads stdout as UTF-8
// and aborts on invalid bytes.
std::string sanitize(const char *s)
{
    std::string out;
    for (const char *p = s; *p != '\0'; ++p) {
        unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 0x20 && c < 0x7f)
            out += static_cast<char>(c);
        else {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\x%02x", c);
            out += buf;
        }
    }
    return out;
}

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

// Turns EV logging off for its lifetime. Serializers may log diagnostics that
// embed raw (non-UTF-8) protocol bytes straight to stdout on odd input; opp_test
// reads stdout as UTF-8 and aborts on invalid bytes, so we mute logging while
// (de)serializing. Thrown exceptions still propagate.
struct LogSilencer
{
    LogLevel saved;
    LogSilencer() : saved(cLog::logLevel) { cLog::logLevel = LOGLEVEL_OFF; }
    ~LogSilencer() { cLog::logLevel = saved; }
};

} // anonymous namespace

// Deserialize (dissect) the packet into typed chunks, re-serialize the result,
// and compare it byte-by-byte with the original content. Also record which
// regions stayed raw (unparsed) for the coverage report.
static bool roundTripOneRecord(Packet *packet, const char *filename, int frame)
{
    Ptr<const BytesChunk> originalBytes;
    Ptr<const BytesChunk> rebuiltBytes;
    bool topLevelRaw = false;
    long rawChunkCount = 0;
    {
        // mute serializer logging during (de)serialization (see LogSilencer)
        LogSilencer silence;

        // reference bytes: the frame exactly as it enters the round-trip (raw wire
        // bytes plus any FCS the caller appended)
        originalBytes = packet->peekDataAsBytes();

        // deserialize: dissect the raw bytes into their most specific typed chunks
        // and reassemble them into a single content chunk (uses PacketProtocolTag)
        CoverageBuilder builder;
        PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), builder);
        packetDissector.dissectPacket(packet);
        topLevelRaw = builder.topLevelRaw;
        rawChunkCount = builder.rawChunkCount;

        // re-serialize
        Packet rebuilt("roundtrip");
        rebuilt.insertAtBack(builder.getContent());
        rebuiltBytes = rebuilt.peekAllAsBytes();
    }

    // note unparsed (raw-bytes) regions
    if (topLevelRaw) {
        std::stringstream ss;
        ss << "TOPLEVEL-RAWBYTES " << filename << " frame " << frame
           << ": outermost protocol not parsed";
        topLevelRawLines.push_back(ss.str());
    }
    rawLeafRegionCount += topLevelRaw ? rawChunkCount - 1 : rawChunkCount;

    if (originalBytes->getChunkLength() != rebuiltBytes->getChunkLength()) {
        EV_WARN << "  Length differs: original " << originalBytes->getChunkLength()
           << " vs rebuilt " << rebuiltBytes->getChunkLength() << "\n";
        return false;
    }
    const auto& a = originalBytes->getBytes();
    const auto& b = rebuiltBytes->getBytes();
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            EV_WARN << "  Bytes differ at offset " << i << "\n"
               << "    original: " << originalBytes->str() << "\n"
               << "    rebuilt:  " << rebuiltBytes->str() << "\n";
            return false;
        }
    }
    return true;
}

bool testPcapSerialization(const char *filename, bool hasFcs)
{
    EV_TRACE << "=== Testing file " << filename << "\n";
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
        EV_TRACE << "  === Frame " << i << "\n";

        // skip truncated records (only the first incl_len bytes were captured)
        const auto& rawData = packet->peekData(Chunk::PF_ALLOW_ALL);
        if (rawData->isIncomplete()) {
            EV_WARN << "  Skipped truncated frame " << i << "\n";
            delete packet;
            continue;
        }

        // a serializer that throws on some real frame must not abort the whole run;
        // report the frame and carry on to the rest of the file/corpus
        try {
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
                EV_TRACE << "  Frame " << i << " is the same\n";
            else {
                EV_WARN << "  Frame " << i << " differs\n";
                allGood = false;
            }
        }
        catch (const std::exception& e) {
            EV_WARN << "  Frame " << i << " EXCEPTION: " << sanitize(e.what()) << "\n";
            allGood = false;
        }
        delete packet;
    }
    reader.closePcap();
    if (allGood)
        EV_WARN << "All frames are the same in file " << filename << "\n";
    else
        EV_WARN << "Some frames differ in file " << filename << "\n";
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

    EV_WARN << "\n=== Serializer coverage (this run) ===\n";
    EV_WARN << "Registered serializers: " << registered.size() << "\n";
    EV_WARN << "Exercised: " << usedSerializers.size() << "\n";
    for (const auto& name : usedSerializers) // std::set -> sorted
        EV_WARN << "  USED " << name << "\n";
    EV_WARN << "Untested (registered but never invoked): " << untested.size() << "\n";
    for (const auto& name : untested)
        EV_WARN << "  UNTESTED " << name << "\n";

    EV_WARN << "\n=== Unparsed (raw-bytes) regions ===\n";
    EV_WARN << "Raw-bytes leaf regions: " << rawLeafRegionCount << " (expected: opaque payloads)\n";
    for (const auto& line : topLevelRawLines)
        EV_WARN << "  " << line << "\n";
    if (topLevelRawLines.empty())
        EV_WARN << "  (none)\n";
}

} // namespace inet
