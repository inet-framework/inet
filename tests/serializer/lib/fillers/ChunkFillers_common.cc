//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the "common" group: the packet-framework raw-data chunks
// (BytesChunk/BitsChunk/ByteCountChunk/BitCountChunk), a handful of small
// protocol-element helper headers, and a few application payloads.
//

#include "../ChunkFillers.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/protocolelement/checksum/header/ChecksumHeader_m.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"
#include "inet/routing/dsdv/DsdvHello_m.h"
#ifdef INET_WITH_VOIPSTREAM
#include "inet/applications/voipstream/VoipStreamPacket_m.h"
#endif

namespace inet {

namespace {

// Turn two FillValues bytes into 16 individual bits (MSB first per byte), so the
// pattern is asymmetric (not all-same, not a palindrome) like every other field.
std::vector<bool> sixteenBits(FillValues& v)
{
    std::vector<bool> bits;
    for (int i = 0; i < 2; i++) {
        uint8_t byte = v.u8();
        for (int b = 7; b >= 0; b--)
            bits.push_back((byte >> b) & 1);
    }
    return bits;
}

} // namespace

void addFillers_common(std::vector<ChunkFiller>& fillers)
{
    // DHCP: every option is written only when it carries a value, and the serializer pads
    // the tail up to the declared chunkLength (so the length cannot be measured from it).
    // The length is therefore accumulated here exactly the way the serializer accounts for
    // it: the 236-byte BOOTP part + the 4-byte magic cookie + the options + the End option.
    fillers.push_back({"inet::DhcpMessage", "", [](Chunk *c) {
        auto p = check_and_cast<DhcpMessage *>(c);
        FillValues v;
        p->setOp(BOOTREQUEST);
        p->setHtype(1); // Ethernet, per the "Assigned Numbers" hardware type list
        p->setHlen(6);  // Ethernet address length
        p->setHops(v.u8());
        p->setXid(v.u32());
        p->setSecs(v.u16());
        p->setBroadcast(true);
        p->setReserved(0); // RFC 2131: MUST BE ZERO
        p->setCiaddr(v.ipv4());
        p->setYiaddr(v.ipv4());
        p->setGiaddr(v.ipv4());
        p->setChaddr(v.mac());
        std::string sname = v.text("srv"), file = v.text("boot");
        p->setSname(sname.c_str());
        p->setFile(file.c_str());
        auto& o = p->getOptionsForUpdate();
        B len = B(236 + 4 + 1);
        o.setMessageType(DHCPREQUEST);
        len += B(3);
        std::string hostName = v.text("host");
        o.setHostName(hostName.c_str());
        len += B(2 + hostName.size());
        o.setParameterRequestListArraySize(2);
        o.setParameterRequestList(0, SUBNET_MASK);
        o.setParameterRequestList(1, ROUTER);
        len += B(2 + 2);
        o.setClientIdentifier(v.mac());
        len += B(2 + 1 + 6);
        o.setRequestedIp(v.ipv4());
        len += B(6);
        o.setSubnetMask(v.ipv4());
        len += B(6);
        o.setRouterArraySize(2);
        o.setRouter(0, v.ipv4());
        o.setRouter(1, v.ipv4());
        len += B(2 + 2 * 4);
        o.setDnsArraySize(2);
        o.setDns(0, v.ipv4());
        o.setDns(1, v.ipv4());
        len += B(2 + 2 * 4);
        o.setNtpArraySize(1);
        o.setNtp(0, v.ipv4());
        len += B(2 + 4);
        o.setServerIdentifier(v.ipv4());
        len += B(6);
        o.setRenewalTime(SimTime(v.u16(), SIMTIME_S)); // written in whole seconds
        len += B(6);
        o.setRebindingTime(SimTime(v.u16(), SIMTIME_S)); // written in whole seconds
        len += B(6);
        o.setLeaseTime(SimTime(v.u16(), SIMTIME_S)); // written in whole seconds
        len += B(6);
        setChunkLength(c, len);
    }});

    // --- packet-framework raw-data chunks: their payload IS their content (no named
    // fields), so give them actual bytes/bits rather than leaving the array empty.
    fillers.push_back({"inet::BytesChunk", "", [](Chunk *c) {
        auto p = check_and_cast<BytesChunk *>(c);
        FillValues v;
        p->setBytes({v.u8(), v.u8(), v.u8(), v.u8(), v.u8()});
    }});
    // A genuinely non-byte-multiple bit count cannot round-trip through this test's
    // MemoryInputStream/MemoryOutputStream, which always materializes/parses a whole
    // number of bytes: a chunk filled with e.g. 7 bits serializes to 1 byte (padded
    // with an implicit 0 bit), the deserialized clone reads that back as 8 real bits
    // (getRemainingLength() is byte-granular), and the engine's separate serialized-
    // length-vs-chunkLength invariant then compares 8 bits against the original 7 and
    // fails -- confirmed empirically with a standalone probe against this build. So,
    // unlike the field-level b(n) values elsewhere (Igmpv3Query::resv, ...), the
    // chunk's own total length here must stay a multiple of 8 bits.
    fillers.push_back({"inet::BitsChunk", "", [](Chunk *c) {
        auto p = check_and_cast<BitsChunk *>(c);
        FillValues v;
        p->setBits(sixteenBits(v));
    }});
    // ByteCountChunkSerializer::deserialize() never recovers the observed data byte --
    // it only verifies the stream against the freshly constructed chunk's *default*
    // getData() ('?' = 0x3F) via readByteRepeatedly(). A data value other than '?'
    // would serialize correctly the first time but the deserialized clone would still
    // report '?', so re-serializing it would produce different bytes and fail the
    // round trip. Leave data at its default (still counted as "filled": '?' is a
    // meaningful, non-0/non-(-1) value) and only set the length.
    fillers.push_back({"inet::ByteCountChunk", "", [](Chunk *c) {
        auto p = check_and_cast<ByteCountChunk *>(c);
        p->setLength(B(4));
    }});
    // Same limitation as ByteCountChunk (BitCountChunkSerializer::deserialize only
    // verifies against the fresh default getData() == false), so data stays at its
    // default; inet::BitCountChunk::data is a permanent knownUnfilled entry.
    fillers.push_back({"inet::BitCountChunk", "", [](Chunk *c) {
        auto p = check_and_cast<BitCountChunk *>(c);
        p->setLength(b(16));
    }});

    // --- EchoPacket: fixed 6-byte header (type/identifier/seqNumber, each 2 bytes on
    // the wire); pin type away from its already-meaningful 0 default so the field is
    // visibly set rather than merely "meaningful by luck".
    fillers.push_back({"inet::EchoPacket", "", [](Chunk *c) {
        auto p = check_and_cast<EchoPacket *>(c);
        FillValues v;
        p->setType(ECHO_PROTOCOL_REPLY);
        p->setIdentifier(v.u16());
        p->setSeqNumber(v.u16());
    }});

    // --- FragmentNumberHeader: fixed 1-byte header, a 7-bit number plus a flag bit.
    fillers.push_back({"inet::FragmentNumberHeader", "", [](Chunk *c) {
        auto p = check_and_cast<FragmentNumberHeader *>(c);
        FillValues v;
        p->setFragmentNumber((int)v.uint(7));
        p->setLastFragment(v.flag());
    }});

    // --- SequenceNumberHeader: fixed 2-byte header (writeUint16Be truncates the
    // int field to its low 16 bits, which is exactly what v.u16() already fits).
    fillers.push_back({"inet::SequenceNumberHeader", "", [](Chunk *c) {
        auto p = check_and_cast<SequenceNumberHeader *>(c);
        FillValues v;
        p->setSequenceNumber((int)v.u16());
    }});

    // --- ChecksumHeader: the .msg leaves chunkLength "set programmatically" (no
    // default), and the serializer switches on the chunkLength itself (1/2/4/8 bytes)
    // to pick the write width -- a content-dependent serializer branch per size, so
    // one variant per width. checksumMode must be COMPUTED or DISABLED; COMPUTED lets
    // the stored value round-trip as-is.
    struct { const char *label; B size; } checksumSizes[] = {
        {"1B", B(1)}, {"2B", B(2)}, {"4B", B(4)}, {"8B", B(8)},
    };
    for (const auto& cs : checksumSizes) {
        fillers.push_back({"inet::ChecksumHeader", cs.label, [size = cs.size](Chunk *c) {
            auto p = check_and_cast<ChecksumHeader *>(c);
            FillValues v;
            p->setChecksumMode(CHECKSUM_COMPUTED);
            // a value that exactly fills the chosen width, so every emitted byte differs
            p->setChecksum(v.uint(size.get<b>()));
            setChunkLength(c, size);
        }});
    }

    // --- ApplicationPacket / EtherAppReq / EtherAppResp: self-sizing payloads. Each
    // writes its own chunkLength as the first 4-byte field, then its fixed fields,
    // then pads with '?' up to the declared chunkLength -- so chunkLength is content-
    // dependent (it is itself part of the content) and must be set above the fixed
    // part to also exercise the padding-write branch.
    fillers.push_back({"inet::ApplicationPacket", "", [](Chunk *c) {
        auto p = check_and_cast<ApplicationPacket *>(c);
        FillValues v;
        p->setSequenceNumber(v.u32());
        setChunkLength(c, B(16)); // fixed part is 8 B (4 length + 4 sequenceNumber)
    }});
    fillers.push_back({"inet::EtherAppReq", "", [](Chunk *c) {
        auto p = check_and_cast<EtherAppReq *>(c);
        FillValues v;
        p->setRequestId((long)v.u32());
        p->setResponseBytes((long)v.u32());
        setChunkLength(c, B(20)); // fixed part is 12 B (4 length + 4 requestId + 4 responseBytes)
    }});
    fillers.push_back({"inet::EtherAppResp", "", [](Chunk *c) {
        auto p = check_and_cast<EtherAppResp *>(c);
        FillValues v;
        p->setRequestId((int)v.u32());
        p->setNumFrames((int)v.u32());
        setChunkLength(c, B(20)); // fixed part is 12 B (4 length + 4 requestId + 4 numFrames)
    }});

#ifdef INET_WITH_VOIPSTREAM
    // --- VoipStreamPacket: the serialized length is the headerLength field (the
    // serializer writes the fixed fields, then pads with '?' up to headerLength), so
    // headerLength/chunkLength must be set above the fixed part. type gates one extra
    // field on the wire (dataLength is written only for VOICE) -- a content-dependent
    // serializer branch, so both variants run; the "silence" variant leaves dataLength
    // at its default (not on the wire for that type), while "voice" fills it, so the
    // field is covered overall. (Guarded by the feature macro, like the old .test's
    // recipe; VoipStreamPacket_m.h itself hard-errors without HAVE_FFMPEG, which the
    // real build's makefrag defines when libavcodec/-format/-util are available.)
    fillers.push_back({"inet::VoipStreamPacket", "silence", [](Chunk *c) {
        auto p = check_and_cast<VoipStreamPacket *>(c);
        FillValues v;
        p->setHeaderLength(30);
        p->setType(SILENCE); // non-VOICE: dataLength is not on the wire
        p->setCodec((int)v.u32());
        p->setSampleBits((short)v.u16());
        p->setSampleRate((int)v.u32());
        p->setTransmitBitrate((int)v.u32());
        p->setSamplesPerPacket((int)v.u32());
        p->setSeqNo(v.u16());
        p->setTimeStamp(v.u32());
        p->setSsrc(v.u32());
        setChunkLength(c, B(30)); // 26 B fixed (non-VOICE) + 4 B of '?' padding
    }});
    fillers.push_back({"inet::VoipStreamPacket", "voice", [](Chunk *c) {
        auto p = check_and_cast<VoipStreamPacket *>(c);
        FillValues v;
        p->setHeaderLength(32);
        p->setType(VOICE); // VOICE: dataLength is written too
        p->setCodec((int)v.u32());
        p->setSampleBits((short)v.u16());
        p->setSampleRate((int)v.u32());
        p->setTransmitBitrate((int)v.u32());
        p->setSamplesPerPacket((int)v.u32());
        p->setSeqNo(v.u16());
        p->setTimeStamp(v.u32());
        p->setSsrc(v.u32());
        p->setDataLength(v.u16());
        setChunkLength(c, B(32)); // 28 B fixed (VOICE) + 4 B of '?' padding
    }});
#endif

    // --- DsdvHello: a small DSDV routing payload, always exactly 16 B on the wire
    // (4 x Ipv4Address/uint32 fields), but the .msg gives it no fixed chunkLength
    // default -- measure it from the serializer.
    fillers.push_back({"inet::DsdvHello", "", [](Chunk *c) {
        auto p = check_and_cast<DsdvHello *>(c);
        FillValues v;
        p->setSrcAddress(v.ipv4());
        p->setSequencenumber(v.u32());
        p->setNextAddress(v.ipv4());
        p->setHopdistance((int)v.u32());
        measureAndSetChunkLength(c);
    }});
}

} // namespace inet
