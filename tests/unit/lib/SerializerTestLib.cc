//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "SerializerTestLib.h"

#include <cstdio>
#include <fstream>
#include <exception>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "inet/common/FcsInd_m.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
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
        // A FieldsChunk built by deserialization caches the original wire bytes, and the
        // serializer replays that cache instead of re-encoding from the parsed fields (see
        // FieldsChunkSerializer). Left as-is, the round-trip below would replay the input
        // and never exercise serialize() -- masking any deserialize/serialize asymmetry.
        // Store a mutable copy with the cache dropped so the re-serialization genuinely
        // re-encodes from the fields.
        Ptr<const Chunk> stored = chunk;
        if (dynamic_cast<const FieldsChunk *>(chunk.get()) != nullptr) {
            auto mutableChunk = makeExclusivelyOwnedMutableChunk(staticPtrCast<const FieldsChunk>(chunk));
            mutableChunk->handleChange();  // clears serializedData (requires a mutable chunk)
            mutableChunk->markImmutable(); // the reassembly below requires immutable chunks
            stored = mutableChunk;
        }
        builder.visitChunk(stored, protocol);
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
// The Ethernet and 802.11 dissectors expect a 4-byte FCS trailer, which real captures
// usually omit -- append a computed one (the 802.11 FCS uses the same CRC-32 as Ethernet).
// A capture may already carry it: the caller states so for Ethernet via hasFcs, and the
// PcapReader detects it per-frame from the radiotap header (the FcsInd tag) for 802.11.
static void appendFcsIfMissing(Packet *packet, bool hasFcs)
{
    auto protocolTag = packet->findTag<PacketProtocolTag>();
    const Protocol *protocol = protocolTag != nullptr ? protocolTag->getProtocol() : nullptr;
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
}

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
        appendFcsIfMissing(packet, hasFcs);

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

// Serializers a pcap round-trip can never exercise: reporting them as SKIPPED
// (rather than UNTESTED) keeps the UNTESTED list to real protocol serializers for
// which a capture could still be added. Four groups:
//
//   1. Generic chunk representations -- the packet "plumbing" (raw byte/bit
//      buffers, composite/slice wrappers), never a distinct protocol on the wire.
//   2. Abstract / dispatch base classes -- the deserializer always yields a
//      concrete subtype (e.g. Ieee80211MacHeader -> data/mgmt/control, BgpHeader ->
//      Open/Update/...), so the base is never the leaf chunk. These are the same
//      dispatch bases serializer_chunk_roundtrip skips.
//   3. INET-internal formats -- research/simplified MAC models, synthetic
//      traffic-generator payloads, and framework helper headers with no real-world
//      capture; the constructed serializer_chunk_roundtrip test covers them instead.
//   4. Physical-layer headers -- a link-layer pcap begins above the PHY, so the PHY
//      preamble/header is never present in a capture.
const std::set<std::string> pcapCoverageSkipTypes = {
    // 1. generic chunk representations
    "inet::BitCountChunk", "inet::BitsChunk", "inet::ByteCountChunk", "inet::BytesChunk",
    "inet::EmptyChunk", "inet::SequenceChunk", "inet::SliceChunk", "inet::StreamBufferChunk",

    // 2. abstract / dispatch base classes (deserializer returns a concrete subtype)
    "inet::ieee80211::Ieee80211DataOrMgmtHeader", "inet::ieee80211::Ieee80211ActionFrame",
    "inet::CfmTlvBase",
    "inet::bgp::BgpHeader", "inet::ospfv2::Ospfv2Packet", "inet::ospfv3::Ospfv3Packet",
    "inet::rtp::RtcpPacket", "inet::aodv::AodvControlPacket", "inet::MobilityHeader",
    "inet::MldMessage", "inet::IgmpQuery", "inet::MrpTlvHeader", "inet::BMacHeaderBase",
    "inet::XMacHeaderBase", "inet::CsmaCaMacHeader", "inet::EthernetControlFrameBase",
    // VLAN/tag "TPID" headers: the packet dissector produces the *TagEpdHeader variant
    // (Ieee8021qTagEpdProtocolDissector etc.); the *TagTpidHeader is the sending/checker
    // simulation-module representation, never popped by a ProtocolDissector.
    "inet::Ieee8021qTagTpidHeader", "inet::Ieee8021aeTagTpidHeader", "inet::Ieee8021rTagTpidHeader",

    // 3. INET-internal formats with no real-world capture (covered by
    //    serializer_chunk_roundtrip): research MACs, synthetic app payloads, helpers
    "inet::CsmaCaMacAckHeader", "inet::CsmaCaMacDataHeader", "inet::CsmaCaMacTrailer",
    "inet::BMacControlFrame", "inet::BMacDataFrameHeader",
    "inet::XMacControlFrame", "inet::XMacDataFrameHeader",
    "inet::AckingMacHeader", "inet::ShortcutMacHeader",
    "inet::EtherAppReq", "inet::EtherAppResp", "inet::ApplicationPacket",
    "inet::VoipStreamPacket", "inet::EchoPacket", "inet::DsdvHello",
    "inet::ChecksumHeader", "inet::FragmentNumberHeader", "inet::SequenceNumberHeader",
    "inet::TransportPseudoHeader",

    // 4. physical-layer headers (a link-layer pcap starts above the PHY)
    "inet::ApskPhyHeader", "inet::GenericPhyHeader", "inet::ShortcutPhyHeader",
    "inet::physicallayer::EthernetPhyHeader", "inet::physicallayer::EthernetFragmentPhyHeader",
    "inet::physicallayer::EthernetPhyHeaderBase",
    "inet::physicallayer::Ieee80211DsssPhyHeader", "inet::physicallayer::Ieee80211ErpOfdmPhyHeader",
    "inet::physicallayer::Ieee80211FhssPhyHeader", "inet::physicallayer::Ieee80211HrDsssPhyHeader",
    "inet::physicallayer::Ieee80211HtPhyHeader", "inet::physicallayer::Ieee80211IrPhyHeader",
    "inet::physicallayer::Ieee80211OfdmPhyHeader", "inet::physicallayer::Ieee80211VhtPhyHeader",
};

// ---- field checks -------------------------------------------------------------------
// Both round-trip tests reproduce the bytes of a frame, so a serializer that reads and
// writes a field in the same wrong place passes them. What catches that is an independent
// decoder: the expected values below come from tshark, and are compared against what the
// dissection of the same frame puts in the fields.

static long fieldChecksMade = 0, fieldChecksFailed = 0;
// set from the environment to list the chunks of every frame the checks look at
static bool fieldCheckVerbose = getenv("FIELDCHECK_VERBOSE") != nullptr;

// What the checks reach and what they leave alone: every field of every chunk the corpus
// dissects, and the subset an expectation names. A reference can only hold what the tool's
// mapping knows about, so the fields it says nothing about are the honest measure of what
// this test does not guard -- and unlike the mapping gap, this side is exact: it comes from
// INET's own descriptors, not from what a second decoder happens to expose. Keyed by the
// class the chunk really is, not by the one an expectation names.
static std::map<std::string, std::set<std::string>> fieldsPresent;
static std::map<std::string, std::set<std::string>> fieldsChecked;

namespace {

// Collects every chunk the dissection yields, in order.
class ChunkCollector : public PacketDissector::ICallback
{
  public:
    std::vector<Ptr<const Chunk>> chunks;

    virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
    virtual void startProtocolDataUnit(const Protocol *protocol) override {}
    virtual void endProtocolDataUnit(const Protocol *protocol) override {}
    virtual void markIncorrect() override {}
    virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override { chunks.push_back(chunk); }
};

// Fields no decoder can have an opinion about, so counting them would only make the
// coverage look worse than it is:
//   * what the simulation kernel declares (the name and class of an object) and what Chunk
//     and FieldsChunk declare (the length, and the flags that say whether the chunk is
//     complete, correct and properly represented) describes the chunk as an object rather
//     than the frame it came from;
//   * what the protocol-agnostic bases declare -- the addresses of NetworkHeaderBase, the
//     ports of TransportHeaderBase -- is a second name for a value the concrete header
//     also declares under its own (srcAddress, srcPort), so checking both would check the
//     same bytes twice;
//   * a "...Mode" field says how INET represents a value (computed, declared correct), and
//     that choice leaves no trace on the wire.
bool isFieldNoDecoderCanSee(cClassDescriptor *descriptor, int field)
{
    static const std::set<std::string> classes = {
        "inet::Chunk", "inet::FieldsChunk", "inet::NetworkHeaderBase", "inet::TransportHeaderBase"
    };
    std::string declaredOn = descriptor->getFieldDeclaredOn(field) ? descriptor->getFieldDeclaredOn(field) : "";
    if (declaredOn.rfind("omnetpp::", 0) == 0 || classes.count(declaredOn) != 0)
        return true;
    std::string name = descriptor->getFieldName(field);
    return name.size() > 4 && name.compare(name.size() - 4, 4, "Mode") == 0;
}

} // namespace

void beginChunkFieldChecks()
{
    fieldChecksMade = 0;
    fieldChecksFailed = 0;
    fieldsPresent.clear();
    fieldsChecked.clear();
}

void checkChunkFields(const char *filename, int frameNumber, bool hasFcs, const std::vector<ChunkFieldCheck>& checks)
{
    ChunkCollector collector;
    {
        LogSilencer silence;
        PcapReader reader;
        reader.openPcap(filename, nullptr);
        Packet *packet = nullptr;
        for (int i = 0; i < frameNumber; i++) {
            delete packet;
            packet = reader.readPacket().second;
            if (packet == nullptr)
                break;
        }
        reader.closePcap();
        if (packet == nullptr) {
            EV_WARN << "  FIELD-CHECK ERROR " << filename << " frame " << frameNumber << ": no such frame\n";
            fieldChecksMade += checks.size();
            fieldChecksFailed += checks.size();
            return;
        }
        appendFcsIfMissing(packet, hasFcs);
        PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), collector);
        packetDissector.dissectPacket(packet);
        delete packet;
    }

    for (const auto& visited : collector.chunks) {
        if (auto descriptor = cClassDescriptor::getDescriptorFor(visited.get())) {
            auto& present = fieldsPresent[visited->getClassName()];
            for (int i = 0; i < descriptor->getFieldCount(); i++)
                if (!isFieldNoDecoderCanSee(descriptor, i))
                    present.insert(descriptor->getFieldName(i));
        }
    }

    if (fieldCheckVerbose) {
        EV_WARN << "  chunks of " << filename << " frame " << frameNumber << ":";
        for (const auto& visited : collector.chunks)
            EV_WARN << "\n    " << visited->getClassName() << ": " << visited->str();
        EV_WARN << "\n";
    }
    for (const auto& check : checks) {
        fieldChecksMade++;
        // The class a frame yields is not always the one the expectation names: an 802.11
        // address sits in a base class that both the data and the management header
        // extend, so an expectation about it holds for either. Match a base class too.
        const Chunk *chunk = nullptr;
        int remaining = check.occurrence;
        for (const auto& visited : collector.chunks) {
            for (auto descriptor = cClassDescriptor::getDescriptorFor(visited.get());
                 descriptor != nullptr; descriptor = descriptor->getBaseClassDescriptor())
            {
                if (check.chunkTypeName == descriptor->getName()) {
                    // a frame may hold several chunks of the same class, a stack of tags
                    // or of labels; the expectation says which one it is about
                    if (remaining-- == 0)
                        chunk = visited.get();
                    break;
                }
            }
            if (chunk != nullptr)
                break;
        }
        if (chunk == nullptr) {
            EV_WARN << "  FIELD-CHECK FAILED " << filename << " frame " << frameNumber
                    << ": no " << check.chunkTypeName << " chunk #" << check.occurrence
                    << " in this frame\n";
            fieldChecksFailed++;
            continue;
        }
        auto descriptor = cClassDescriptor::getDescriptorFor(chunk);
        int field = descriptor != nullptr ? descriptor->findField(check.fieldName.c_str()) : -1;
        if (field == -1) {
            EV_WARN << "  FIELD-CHECK FAILED " << filename << " frame " << frameNumber
                    << ": " << check.chunkTypeName << " has no field '" << check.fieldName << "'\n";
            fieldChecksFailed++;
            continue;
        }
        // the field was looked at, whatever the value turns out to be; record it under the
        // name the descriptor gives it, which is what the coverage report enumerates
        fieldsChecked[chunk->getClassName()].insert(descriptor->getFieldName(field));
        std::string value = descriptor->getFieldValueAsString(toAnyPtr(const_cast<Chunk *>(chunk)), field, 0);
        if (value != check.expectedValue) {
            EV_WARN << "  FIELD-CHECK FAILED " << filename << " frame " << frameNumber << ": "
                    << check.chunkTypeName << "#" << check.occurrence << "." << check.fieldName << " is " << sanitize(value.c_str())
                    << ", expected " << check.expectedValue << "\n";
            fieldChecksFailed++;
        }
    }
}

void checkChunkFieldsFromReference(const char *filename, bool hasFcs)
{
    std::string referenceName = filename;
    referenceName = referenceName.substr(0, referenceName.rfind('.')) + ".fields";
    std::ifstream reference(referenceName);
    if (!reference) {
        EV_WARN << "  FIELD-CHECK ERROR: no field reference " << referenceName
                << " (write it with tests/unit/tools/mkfieldrefs.py)\n";
        fieldChecksMade++;
        fieldChecksFailed++;
        return;
    }
    // the rows are grouped by frame, so collect a frame's expectations and check them
    // together: dissecting the frame once for all of them is the whole point of the file
    int currentFrame = -1;
    std::vector<ChunkFieldCheck> checks;
    std::string line;
    while (std::getline(reference, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::vector<std::string> columns;
        size_t start = 0;
        for (size_t tab = line.find('\t'); ; tab = line.find('\t', start)) {
            columns.push_back(line.substr(start, tab == std::string::npos ? tab : tab - start));
            if (tab == std::string::npos)
                break;
            start = tab + 1;
        }
        if (columns.size() < 5) {
            EV_WARN << "  FIELD-CHECK ERROR: malformed line in " << referenceName << ": " << line << "\n";
            fieldChecksMade++;
            fieldChecksFailed++;
            continue;
        }
        int frame = atoi(columns[0].c_str());
        if (frame != currentFrame && currentFrame != -1) {
            checkChunkFields(filename, currentFrame, hasFcs, checks);
            checks.clear();
        }
        currentFrame = frame;
        checks.push_back({ columns[1], atoi(columns[2].c_str()), columns[3], columns[4] });
    }
    if (!checks.empty())
        checkChunkFields(filename, currentFrame, hasFcs, checks);
}

void reportChunkFieldChecks()
{
    EV_WARN << "\n=== SUMMARY ===\n";
    EV_WARN << "Field checks: " << fieldChecksMade << " made, " << fieldChecksFailed << " failed\n";

    long present = 0, checked = 0, coveredClasses = 0;
    for (const auto& [className, fields] : fieldsPresent) {
        auto it = fieldsChecked.find(className);
        present += fields.size();
        checked += it == fieldsChecked.end() ? 0 : it->second.size();
        if (it != fieldsChecked.end() && it->second.size() == fields.size())
            coveredClasses++;
    }
    EV_WARN << "Field coverage: " << checked << " of " << present << " fields of "
            << fieldsPresent.size() << " dissected chunk classes are checked ("
            << coveredClasses << " classes fully)\n";
    EV_WARN << "Fields no expectation names (a capture reaches the chunk, nothing checks the field):\n";
    for (const auto& [className, fields] : fieldsPresent) {
        auto it = fieldsChecked.find(className);
        std::string unchecked;
        for (const auto& field : fields)
            if (it == fieldsChecked.end() || it->second.count(field) == 0)
                unchecked += (unchecked.empty() ? "" : ", ") + field;
        if (!unchecked.empty())
            EV_WARN << "  " << className << ": " << unchecked << "\n";
    }
}

} // namespace inet
