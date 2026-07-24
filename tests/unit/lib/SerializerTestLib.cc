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

#include "inet/common/FcsInd_m.h"
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
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

// accumulated coverage state (WP2), spanning all files of one run
static std::set<std::string> usedSerializers;
static std::set<std::string> usedSerializerClasses; // dispatched serializer *classes* (code-level coverage)
static long rawLeafRegionCount = 0;
static std::vector<std::string> topLevelRawLines;
// aggregate result counters (reset by beginSerializerCoverage, printed by reportSerializerCoverage)
static long filesTested = 0, filesClean = 0;
static long framesSame = 0, framesDiffer = 0, framesException = 0, framesTruncated = 0;

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
    const Protocol *lastProtocol = nullptr; // deepest protocol entered (for diagnostics)
    b bytesParsed = b(0); // total length successfully dissected so far (= offset of the next chunk)

    const Ptr<const Chunk> getContent() { return builder.getContent(); }

    virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
    virtual void startProtocolDataUnit(const Protocol *protocol) override { if (protocol != nullptr) lastProtocol = protocol; }
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
        bytesParsed += chunk->getChunkLength();
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

// A space-separated hex dump of a packet's bytes (capped), so a failing frame can
// be inspected/reproduced.
std::string hexDump(Packet *packet, size_t maxBytes = 128)
{
    const auto& bytes = packet->peekDataAsBytes()->getBytes();
    std::string out;
    char buf[4];
    size_t n = bytes.size() < maxBytes ? bytes.size() : maxBytes;
    for (size_t i = 0; i < n; i++) {
        snprintf(buf, sizeof(buf), "%02x ", bytes[i]);
        out += buf;
    }
    if (bytes.size() > maxBytes)
        out += "... (" + std::to_string(bytes.size() - maxBytes) + " more bytes)";
    return out;
}

} // anonymous namespace

// Convenience: " of protocol 'X'" if a protocol was entered, else "".
static std::string protocolContext(const Protocol *protocol)
{
    return protocol != nullptr ? std::string(" of protocol '") + protocol->getName() + "'" : std::string();
}

// Deserialize (dissect) the packet into typed chunks, re-serialize the result,
// and compare it byte-by-byte with the original content. Records raw (unparsed)
// regions for the coverage report, and reports any exception or mismatch with
// enough context (phase, deepest protocol, frame hex) to locate the culprit.
// Returns true iff the frame round-trips identically.
static bool roundTripOneRecord(Packet *packet, const char *filename, int frame, bool hasFcs)
{
    Ptr<const BytesChunk> originalBytes;
    Ptr<const BytesChunk> rebuiltBytes;
    CoverageBuilder builder; // declared out here so lastProtocol survives an exception
    const char *phase = "deserialize";
    try {
        // mute serializer logging during (de)serialization (see LogSilencer)
        LogSilencer silence;

        // real captures usually omit the FCS, but the Ethernet and 802.11 dissectors
        // expect a 4-byte FCS trailer -- append a computed one (the 802.11 FCS uses the
        // same CRC-32 as Ethernet), matching the input on both sides. A capture may
        // already carry the FCS: the caller states so for Ethernet via hasFcs, and the
        // PcapReader detects it per-frame from the radiotap header (the FcsInd tag) for
        // 802.11, so a frame already carrying its FCS is left untouched.
        auto protocolTag = packet->findTag<PacketProtocolTag>();
        const Protocol *protocol = protocolTag != nullptr ? protocolTag->getProtocol() : nullptr;
        // the PcapReader knows the FCS presence for some link types (radiotap, an
        // FCS-explicit 802.15.4 link type) and reports it via FcsInd; when it does not,
        // fall back to the caller-supplied hasFcs (Ethernet, raw 802.11)
        auto fcsInd = packet->findTag<FcsInd>();
        bool frameHasFcs = fcsInd != nullptr ? fcsInd->getHasFcs() : hasFcs;
        if (!frameHasFcs && protocol == &Protocol::ethernetMac) {
            const auto& data = packet->peekDataAsBytes();
            auto fcsChunk = makeShared<EthernetFcs>();
            fcsChunk->setFcs(ethernetFcs(data->getBytes()));
            fcsChunk->setFcsMode(FCS_COMPUTED);
            packet->insertAtBack(fcsChunk);
        }
        else if (!frameHasFcs && protocol == &Protocol::ieee80211Mac) {
            const auto& data = packet->peekDataAsBytes();
            auto fcsChunk = makeShared<ieee80211::Ieee80211MacTrailer>();
            fcsChunk->setFcs(ethernetFcs(data->getBytes()));
            fcsChunk->setFcsMode(FCS_COMPUTED);
            packet->insertAtBack(fcsChunk);
        }

        // reference bytes: the frame exactly as it enters the round-trip
        originalBytes = packet->peekDataAsBytes();

        // deserialize: dissect into typed chunks and reassemble the content
        PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), builder);
        packetDissector.dissectPacket(packet);

        // re-serialize
        phase = "re-serialize";
        Packet rebuilt("roundtrip");
        rebuilt.insertAtBack(builder.getContent());
        rebuiltBytes = rebuilt.peekAllAsBytes();
    }
    catch (const std::exception& e) {
        // during deserialize, the length parsed so far is the byte offset where the
        // failing chunk begins; during re-serialize the whole packet was already parsed
        std::string where = protocolContext(builder.lastProtocol);
        if (std::string(phase) == "deserialize")
            where += " at byte offset " + std::to_string(builder.bytesParsed.get<b>() / 8);
        EV_WARN << "  Frame " << frame << " EXCEPTION during " << phase << where
                << ": " << sanitize(e.what()) << "\n"
                << "    frame bytes: " << hexDump(packet) << "\n";
        framesException++;
        return false;
    }

    // note unparsed (raw-bytes) regions
    if (builder.topLevelRaw) {
        std::stringstream ss;
        ss << "TOPLEVEL-RAWBYTES " << filename << " frame " << frame
           << ": outermost protocol not parsed";
        topLevelRawLines.push_back(ss.str());
    }
    rawLeafRegionCount += builder.topLevelRaw ? builder.rawChunkCount - 1 : builder.rawChunkCount;

    if (originalBytes->getChunkLength() != rebuiltBytes->getChunkLength()) {
        EV_WARN << "  Frame " << frame << " differs: length original "
                << originalBytes->getChunkLength() << " vs rebuilt " << rebuiltBytes->getChunkLength()
                << protocolContext(builder.lastProtocol) << "\n"
                << "    frame bytes: " << hexDump(packet) << "\n";
        framesDiffer++;
        return false;
    }
    const auto& a = originalBytes->getBytes();
    const auto& b = rebuiltBytes->getBytes();
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            EV_WARN << "  Frame " << frame << " differs at byte offset " << i
                    << protocolContext(builder.lastProtocol) << "\n"
                    << "    original: " << originalBytes->str() << "\n"
                    << "    rebuilt:  " << rebuiltBytes->str() << "\n";
            framesDiffer++;
            return false;
        }
    }
    framesSame++;
    return true;
}

bool testPcapSerialization(const char *filename, bool hasFcs)
{
    EV_TRACE << "=== Testing file " << filename << "\n";
    filesTested++;
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
            framesTruncated++;
            delete packet;
            continue;
        }

        // roundTripOneRecord reports any exception/mismatch (with context) and never
        // throws for a serializer failure; the fallback catch here guards the rest.
        try {
            if (roundTripOneRecord(packet, filename, i, hasFcs))
                EV_TRACE << "  Frame " << i << " is the same\n";
            else
                allGood = false;
        }
        catch (const std::exception& e) {
            EV_WARN << "  Frame " << i << " EXCEPTION (uncaught): " << sanitize(e.what()) << "\n";
            framesException++;
            allGood = false;
        }
        delete packet;
    }
    reader.closePcap();
    if (allGood) {
        filesClean++;
        EV_WARN << "All frames are the same in file " << filename << "\n";
    }
    else
        EV_WARN << "Some frames differ in file " << filename << "\n";
    return allGood;
}

void beginSerializerCoverage()
{
    usedSerializers.clear();
    rawLeafRegionCount = 0;
    topLevelRawLines.clear();
    filesTested = filesClean = 0;
    framesSame = framesDiffer = framesException = framesTruncated = 0;
    usedSerializerClasses.clear();
    ChunkSerializerRegistry::getInstance().setUsedSerializerRecorder(&usedSerializers);
    ChunkSerializerRegistry::getInstance().setUsedSerializerClassRecorder(&usedSerializerClasses);
}

void reportSerializerCoverage(const std::set<std::string>& skipTypes)
{
    auto& registry = ChunkSerializerRegistry::getInstance();
    registry.setUsedSerializerRecorder(nullptr); // stop recording
    registry.setUsedSerializerClassRecorder(nullptr);

    const auto& registered = registry.getRegisteredTypeNames();
    std::vector<std::string> untested;
    std::vector<std::string> skipped;
    for (const auto& name : registered) { // std::set -> sorted
        if (usedSerializers.find(name) != usedSerializers.end())
            continue;
        if (skipTypes.find(name) != skipTypes.end())
            skipped.push_back(name); // generic chunk representation, not pcap-testable
        else
            untested.push_back(name);
    }

    EV_WARN << "\n=== Serializer coverage (this run) ===\n";
    EV_WARN << "Registered serializers: " << registered.size() << "\n";
    EV_WARN << "Exercised: " << usedSerializers.size() << "\n";
    for (const auto& name : usedSerializers) // std::set -> sorted
        EV_WARN << "  USED " << name << "\n";
    EV_WARN << "Skipped (not pcap-testable -- generic chunks, dispatch bases, INET-internal formats, PHY headers): " << skipped.size() << "\n";
    for (const auto& name : skipped)
        EV_WARN << "  SKIPPED " << name << "\n";
    EV_WARN << "Untested (registered but never invoked): " << untested.size() << "\n";
    for (const auto& name : untested)
        EV_WARN << "  UNTESTED " << name << "\n";

    // Serializer *class*-level coverage: many chunk types share one serializer class, so a
    // skip-listed type is still covered when a non-skipped sibling exercises the same class.
    // A class no type invokes is genuinely untested code (an abstract-base or dead serializer).
    const auto& registeredClasses = registry.getRegisteredSerializerClassNames();
    std::vector<std::string> uninvokedClasses;
    for (const auto& cls : registeredClasses) // std::set -> sorted
        if (usedSerializerClasses.find(cls) == usedSerializerClasses.end())
            uninvokedClasses.push_back(cls);
    EV_WARN << "\n=== Serializer class coverage (code-level; one class often serves many types) ===\n";
    EV_WARN << "Serializer classes: " << registeredClasses.size() << " registered, "
            << usedSerializerClasses.size() << " invoked, " << uninvokedClasses.size() << " never invoked\n";
    for (const auto& cls : uninvokedClasses)
        EV_WARN << "  NEVER-INVOKED " << cls << "\n";

    EV_WARN << "\n=== Unparsed (raw-bytes) regions ===\n";
    EV_WARN << "Raw-bytes leaf regions: " << rawLeafRegionCount << " (expected: opaque payloads)\n";
    for (const auto& line : topLevelRawLines)
        EV_WARN << "  " << line << "\n";
    if (topLevelRawLines.empty())
        EV_WARN << "  (none)\n";

    long framesTotal = framesSame + framesDiffer + framesException + framesTruncated;
    EV_WARN << "\n=== SUMMARY ===\n";
    EV_WARN << "Serializers: " << registered.size() << " registered, " << usedSerializers.size()
            << " used, " << skipped.size() << " skipped, " << untested.size() << " untested\n";
    EV_WARN << "Files:  " << filesTested << " tested, " << filesClean << " clean, "
            << (filesTested - filesClean) << " with differences\n";
    EV_WARN << "Frames: " << framesTotal << " total, " << framesSame << " same, " << framesDiffer
            << " differ, " << framesException << " error, " << framesTruncated << " truncated\n";
}

} // namespace inet
