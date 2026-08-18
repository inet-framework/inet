//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk filler for inet::sctp::SctpHeader: a container of owned SctpChunk objects
// (cPacket-based, not Chunk-based -- they have no serializer-registry entry of their
// own), dispatched in SctpHeaderSerializer by a big switch over the chunk type. One
// variant per chunk type the serializer supports, so every switch branch runs.
//
// Chunk-type and parameter-type numbers are used as plain literals with a comment
// (as the old data-driven recipe did for SctpCookieAckChunk's type 11) rather than
// pulling in SctpAssociation.h -- a heavy internal header the serializer itself does
// not depend on.
//

#include "../ChunkFillers.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpHeader_m.h"

namespace inet {

namespace {

using namespace sctp;

// The common 12-byte SCTP header shared by every chunk variant below.
void fillSctpHeaderBase(SctpHeader *h, FillValues& v)
{
    h->setSrcPort(v.u16());
    h->setDestPort(v.u16());
    h->setVTag(v.u32());
    // checksum/checksumOk are not wire-written by the serializer (it always computes
    // a fresh crc32c on serialize, and checksumOk is deserialize's verification
    // result) but are real descriptor fields, so give them non-default values too.
    h->setChecksum(v.u32());
    h->setChecksumOk(true);
    // headerLength (.msg default 12) is not read by the serializer, but IS the
    // running total appendSctpChunks() (below) mutates internally to compute
    // chunkLength as chunks are appended -- do not touch it here (its default is
    // already a "meaningful" non-zero, non-sentinel value, so the coverage audit
    // does not flag it either way).
}

// SctpChunk's own base fields (flags/length/chunkName) are not read by any of the
// serializer's per-type branches, but are real descriptor fields on every chunk.
void fillSctpChunkBase(SctpChunk *c, FillValues& v)
{
    c->setFlags(v.u32());
    c->setLength(v.u16());
    c->setChunkNameArraySize(2);
    c->setChunkName(0, 'a' + (v.u8() % 26));
    c->setChunkName(1, 'a' + (v.u8() % 26));
}

// A fully filled SctpCookie: the structured encoding COOKIE_ECHO's serializer emits
// when its raw cookie[] array is empty (state_cookie built straight from these
// fields), and INIT-ACK's stateCookie pointer field (there only for coverage --
// INIT-ACK's own variant below carries its cookie the other way, via cookie[], see
// its comment). creationTimeRaw is set non-zero so the wire creationTime comes from
// it rather than from a float round trip through simtime_t.
SctpCookie *makeSctpCookie(FillValues& v)
{
    auto k = new SctpCookie();
    k->setCreationTime(v.time());
    k->setCreationTimeRaw(v.u32());
    k->setLocalTag(v.u32());
    k->setPeerTag(v.u32());
    k->setLocalTieTagArraySize(32);
    k->setPeerTieTagArraySize(32);
    for (int i = 0; i < 32; i++) {
        k->setLocalTieTag(i, v.u8());
        k->setPeerTieTag(i, v.u8());
    }
    k->setLength(v.u32());
    return k;
}

} // namespace

void addFillers_sctp(std::vector<ChunkFiller>& fillers)
{
    // --- DATA (type 0): one SctpSimpleMessage payload of 4 bytes (a multiple of 4,
    // so ADD_PADDING is a no-op and dc->length == the physical byte count).
    fillers.push_back({"inet::sctp::SctpHeader", "data", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpDataChunk("DATA");
        ch->setSctpChunkType(0); // DATA
        fillSctpChunkBase(ch, v);
        ch->setEBit(true);
        ch->setBBit(false);
        ch->setUBit(true);
        ch->setIBit(false);
        ch->setTsn(v.u32());
        ch->setSid(v.u16());
        ch->setSsn(v.u16());
        ch->setPpid(v.u32());
        ch->setEnqueuingTime(v.time());
        ch->setFirstSendTime(v.time());
        ch->setByteLength(16); // SCTP_DATA_CHUNK_LENGTH, before encapsulate() adds the payload
        auto smsg = new SctpSimpleMessage("data");
        smsg->setDataLen(4);
        smsg->setDataArraySize(4);
        smsg->setData(0, v.u8());
        smsg->setData(1, v.u8());
        smsg->setData(2, v.u8());
        smsg->setData(3, v.u8());
        smsg->setCreationTime(v.time());
        smsg->setEncaps(true);
        smsg->setByteLength(4);
        ch->encapsulate(smsg); // bumps ch's own byteLength to 16+4 == dc->length on the wire
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 16 + 4));
    }});

    // --- INIT (type 1): the supported-address-types, forward-TSN, one IPv4 address
    // and one unrecognized parameter (INIT's serializer copies unrecognizedParameters
    // verbatim onto the wire and its deserializer always stores an unrecognized TLV
    // back into that same array unconditionally, so this one genuinely round-trips).
    fillers.push_back({"inet::sctp::SctpHeader", "init", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpInitChunk("INIT");
        ch->setSctpChunkType(1); // INIT
        fillSctpChunkBase(ch, v);
        ch->setInitTag(v.u32());
        ch->setA_rwnd(v.u32());
        ch->setNoOutStreams(v.u16());
        ch->setNoInStreams(v.u16());
        ch->setInitTsn(v.u32());
        ch->setIpv4Supported(true);
        ch->setIpv6Supported(false); // sup_addr parameter: 8 B
        ch->setForwardTsn(true);     // forward-TSN-supported parameter: 4 B
        ch->setAddressesArraySize(1);
        ch->setAddresses(0, v.l3Ipv4()); // one IPv4 address parameter: 8 B
        // an unrecognized parameter, preserved verbatim by both directions: a
        // type with the skip bit (0x8000) set, a self-consistent length (the
        // whole TLV, header included), and 4 bytes of payload -- 8 B total.
        ch->setUnrecognizedParametersArraySize(8);
        ch->setUnrecognizedParameters(0, 0xC0);
        ch->setUnrecognizedParameters(1, 0xAB);
        ch->setUnrecognizedParameters(2, 0x00);
        ch->setUnrecognizedParameters(3, 0x08);
        ch->setUnrecognizedParameters(4, v.u8());
        ch->setUnrecognizedParameters(5, v.u8());
        ch->setUnrecognizedParameters(6, v.u8());
        ch->setUnrecognizedParameters(7, v.u8());
        // msg_rwnd/sctpChunkTypes/sepChunks/hmacTypes/random back the AUTH-related
        // parameters (CHUNKS/HMAC_ALGO/RANDOM), which this variant does not exercise
        // (see the report); left at default.
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 8 + 4 + 8 + 8));
    }});
    // an INIT advertising an IPv6 address: its address parameter is 20 octets and the
    // words are read back in network byte order
    fillers.push_back({"inet::sctp::SctpHeader", "init-v6", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpInitChunk("INIT");
        ch->setSctpChunkType(1); // INIT
        fillSctpChunkBase(ch, v);
        ch->setInitTag(v.u32());
        ch->setA_rwnd(v.u32());
        ch->setNoOutStreams(v.u16());
        ch->setNoInStreams(v.u16());
        ch->setInitTsn(v.u32());
        ch->setIpv4Supported(false);
        ch->setIpv6Supported(true); // sup_addr parameter: 8 B
        ch->setAddressesArraySize(1);
        ch->setAddresses(0, v.l3Ipv6()); // one IPv6 address parameter: 20 B
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 8 + 20));
    }});
    // the AUTH-block parameters, only ever written together: SUPPORTED_EXTENSIONS is
    // gated on sepChunks[] alone, RANDOM/CHUNKS/HMAC_ALGO together on hmacTypes[]
    // alone. Four elements each keeps every one of the four parameters at exactly
    // 8 B (a multiple of 4, so ADD_PADDING is a no-op): SUPPORTED_EXTENSIONS is
    // 4 + 4 sepChunks, RANDOM is 4 + 4 random bytes, CHUNKS is 4 + 4 chunk types,
    // HMAC_ALGO is 4 + 2*2 HMAC types.
    fillers.push_back({"inet::sctp::SctpHeader", "init-auth", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpInitChunk("INIT");
        ch->setSctpChunkType(1); // INIT
        fillSctpChunkBase(ch, v);
        ch->setInitTag(v.u32());
        ch->setA_rwnd(v.u32());
        ch->setNoOutStreams(v.u16());
        ch->setNoInStreams(v.u16());
        ch->setInitTsn(v.u32());
        ch->setSepChunksArraySize(4);
        for (int i = 0; i < 4; i++)
            ch->setSepChunks(i, v.u8());
        ch->setRandomArraySize(4);
        for (int i = 0; i < 4; i++)
            ch->setRandom(i, v.u8());
        ch->setSctpChunkTypesArraySize(4);
        for (int i = 0; i < 4; i++)
            ch->setSctpChunkTypes(i, v.u8());
        ch->setHmacTypesArraySize(2); // the gate: > 0 wires RANDOM/CHUNKS/HMAC_ALGO
        for (int i = 0; i < 2; i++)
            ch->setHmacTypes(i, v.u16());
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 8 + 8 + 8 + 8));
    }});

    // --- INIT_ACK (type 2): same address/forward-TSN parameters as INIT, a cookie
    // via the raw cookie[] bytes path (simpler than building a stateCookie the
    // serializer would also accept), and an unrecognized parameter -- SEE THE REPORT:
    // unlike INIT, this direction's wrap-as-UNRECOGNIZED_PARAMETER re-emission is
    // never recovered by the deserializer's report-bit check, so this specific
    // variant is expected to FAIL the byte round trip (a genuine INET bug, not
    // fixed here).
    fillers.push_back({"inet::sctp::SctpHeader", "initack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpInitAckChunk("INIT_ACK");
        ch->setSctpChunkType(2); // INIT_ACK
        fillSctpChunkBase(ch, v);
        ch->setInitTag(v.u32());
        ch->setA_rwnd(v.u32());
        ch->setNoOutStreams(v.u16());
        ch->setNoInStreams(v.u16());
        ch->setInitTsn(v.u32());
        ch->setIpv4Supported(true);
        ch->setIpv6Supported(false); // sup_addr parameter: 8 B
        ch->setForwardTsn(true);     // forward-TSN-supported parameter: 4 B
        ch->setAddressesArraySize(1);
        ch->setAddresses(0, v.l3Ipv4()); // one IPv4 address parameter: 8 B
        ch->setUnrecognizedParametersArraySize(8); // see the report: does not round-trip
        ch->setUnrecognizedParameters(0, 0xC0);
        ch->setUnrecognizedParameters(1, 0xAB);
        ch->setUnrecognizedParameters(2, 0x00);
        ch->setUnrecognizedParameters(3, 0x08);
        ch->setUnrecognizedParameters(4, v.u8());
        ch->setUnrecognizedParameters(5, v.u8());
        ch->setUnrecognizedParameters(6, v.u8());
        ch->setUnrecognizedParameters(7, v.u8());
        ch->setCookieArraySize(8); // raw-cookie-bytes path: cookie parameter = 4 + 8 B
        for (int i = 0; i < 8; i++)
            ch->setCookie(i, (char)v.u8());
        ch->setStateCookie(makeSctpCookie(v)); // unused by this path; filled for coverage
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 8 + 4 + 8 + 12 + 12));
    }});
    // the same AUTH-block trio as INIT's "init-auth" variant above, plus one IPv6
    // address (the INIT_PARAM_IPV6 branch, which does not need its own INIT sibling
    // -- that one already has "init-v6") and a raw-bytes cookie (as in "initack"
    // above; the mandatory cookie parameter is always present regardless).
    fillers.push_back({"inet::sctp::SctpHeader", "initack-auth", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpInitAckChunk("INIT_ACK");
        ch->setSctpChunkType(2); // INIT_ACK
        fillSctpChunkBase(ch, v);
        ch->setInitTag(v.u32());
        ch->setA_rwnd(v.u32());
        ch->setNoOutStreams(v.u16());
        ch->setNoInStreams(v.u16());
        ch->setInitTsn(v.u32());
        ch->setAddressesArraySize(1);
        ch->setAddresses(0, v.l3Ipv6()); // one IPv6 address parameter: 20 B
        ch->setSepChunksArraySize(4); // SUPPORTED_EXTENSIONS parameter: 4 + 4 = 8 B
        for (int i = 0; i < 4; i++)
            ch->setSepChunks(i, v.u8());
        ch->setRandomArraySize(4); // RANDOM parameter: 4 + 4 = 8 B
        for (int i = 0; i < 4; i++)
            ch->setRandom(i, v.u8());
        ch->setSctpChunkTypesArraySize(4); // CHUNKS parameter: 4 + 4 = 8 B
        for (int i = 0; i < 4; i++)
            ch->setSctpChunkTypes(i, v.u8());
        ch->setHmacTypesArraySize(2); // HMAC_ALGO parameter: 4 + 2*2 = 8 B; the gate
        for (int i = 0; i < 2; i++)
            ch->setHmacTypes(i, v.u16());
        ch->setCookieArraySize(8); // raw-cookie-bytes path: cookie parameter = 4 + 8 B
        for (int i = 0; i < 8; i++)
            ch->setCookie(i, (char)v.u8());
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 20 + 8 + 8 + 8 + 8 + 12));
    }});

    // --- SACK (type 3): one gap-ack block and one duplicate TSN (each a multiple of
    // 4 B, so no chunk padding).
    fillers.push_back({"inet::sctp::SctpHeader", "sack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpSackChunk("SACK");
        ch->setSctpChunkType(3); // SACK
        fillSctpChunkBase(ch, v);
        uint32_t cumTsnAck = 1000;
        ch->setCumTsnAck(cumTsnAck);
        ch->setA_rwnd(v.u32());
        ch->setNumGaps(1);
        ch->setNumNrGaps(0);
        ch->setNumDupTsns(1);
        ch->setIsNrSack(false);
        ch->setGapStartArraySize(1);
        ch->setGapStart(0, cumTsnAck + 5);
        ch->setGapStopArraySize(1);
        ch->setGapStop(0, cumTsnAck + 10);
        ch->setDupTsnsArraySize(1);
        ch->setDupTsns(0, v.u32());
        ch->setSackSeqNum(v.u32());
        // nrGapStart/nrGapStop/msg_rwnd/dacPacketsRcvd/nrSubtractRGaps are for the
        // NR-SACK branch (see the "nr" variant below); filled for coverage.
        ch->setNrGapStartArraySize(1);
        ch->setNrGapStart(0, v.u32());
        ch->setNrGapStopArraySize(1);
        ch->setNrGapStop(0, v.u32());
        ch->setMsg_rwnd(v.u32());
        ch->setDacPacketsRcvd(v.u8());
        ch->setNrSubtractRGaps(true);
        ch->setByteLength(16 + 4 + 4); // SCTP_SACK_CHUNK_LENGTH + one gap + one dup TSN
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 16 + 4 + 4));
    }});

    // --- NR_SACK (type 16): same SctpSackChunk class, discriminated by chunk type
    // and isNrSack. SEE THE REPORT: SctpHeaderSerializer's deserializer has no case
    // for this chunk type at all (only serialize() does), so this variant is
    // expected to FAIL -- the chunk is silently dropped instead of round-tripped.
    fillers.push_back({"inet::sctp::SctpHeader", "nrsack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpSackChunk("NR_SACK");
        ch->setSctpChunkType(16); // NR_SACK
        fillSctpChunkBase(ch, v);
        uint32_t cumTsnAck = 2000;
        ch->setCumTsnAck(cumTsnAck);
        ch->setA_rwnd(v.u32());
        ch->setNumGaps(1);
        ch->setNumNrGaps(1);
        ch->setNumDupTsns(1);
        ch->setIsNrSack(true);
        ch->setGapStartArraySize(1);
        ch->setGapStart(0, cumTsnAck + 5);
        ch->setGapStopArraySize(1);
        ch->setGapStop(0, cumTsnAck + 10);
        ch->setNrGapStartArraySize(1);
        ch->setNrGapStart(0, cumTsnAck + 15);
        ch->setNrGapStopArraySize(1);
        ch->setNrGapStop(0, cumTsnAck + 20);
        ch->setDupTsnsArraySize(1);
        ch->setDupTsns(0, v.u32());
        ch->setSackSeqNum(v.u32());
        ch->setMsg_rwnd(v.u32());
        ch->setDacPacketsRcvd(v.u8());
        ch->setNrSubtractRGaps(true);
        ch->setByteLength(20 + 4 + 4 + 4); // SCTP_NRSACK_CHUNK_LENGTH + gap + nr-gap + dup TSN
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 20 + 4 + 4 + 4));
    }});

    // --- HEARTBEAT (type 4). SEE THE REPORT: the serializer's sender-info encoding
    // writes the IPv4 address at a hand-computed offset but the time field through a
    // union sized for the (larger) IPv6 variant, so getTimeField() lands past the
    // bytes actually emitted and is silently dropped; then the deserializer treats
    // the whole payload as opaque "info" bytes and never reconstructs remoteAddr,
    // so a *second* serialize on the deserialized chunk hits an unconditional
    // ASSERT(infolen != 0), which throws a cRuntimeError. This variant is expected
    // to FAIL with that exception (ASSERT throws rather than aborting the process).
    fillers.push_back({"inet::sctp::SctpHeader", "heartbeat", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpHeartbeatChunk("HEARTBEAT");
        ch->setSctpChunkType(4); // HEARTBEAT
        fillSctpChunkBase(ch, v);
        ch->setRemoteAddr(v.l3Ipv4());
        ch->setTimeField(SimTime(v.u16(), SIMTIME_S)); // the info carries whole seconds
        ch->setByteLength(4 + 4 + 8 + 4); // chunk hdr + info hdr + v4-address TLV + time
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 4 + 8 + 4));
    }});
    // the Heartbeat Info parameter is opaque (RFC 4960 4.2): a peer may put anything
    // there, and it must travel back verbatim
    fillers.push_back({"inet::sctp::SctpHeader", "heartbeat-info", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpHeartbeatChunk("HEARTBEAT");
        ch->setSctpChunkType(4); // HEARTBEAT
        fillSctpChunkBase(ch, v);
        ch->setInfoArraySize(8);
        for (int i = 0; i < 8; i++)
            ch->setInfo(i, v.u8());
        ch->setByteLength(4 + 4 + 8); // chunk hdr + info hdr + info
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 4 + 8));
    }});

    // --- HEARTBEAT_ACK (type 5): the getInfoArraySize()>0 branch, which (unlike
    // HEARTBEAT above) writes/reads the info bytes tightly packed and round-trips
    // cleanly; remoteAddr/timeField are filled too (they back the *other* branch,
    // taken only when info[] is empty) but are not on the wire for this variant.
    fillers.push_back({"inet::sctp::SctpHeader", "heartbeatack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpHeartbeatAckChunk("HEARTBEAT_ACK");
        ch->setSctpChunkType(5); // HEARTBEAT_ACK
        fillSctpChunkBase(ch, v);
        ch->setRemoteAddr(v.l3Ipv4());
        ch->setTimeField(v.time());
        ch->setInfoArraySize(4);
        ch->setInfo(0, v.u8());
        ch->setInfo(1, v.u8());
        ch->setInfo(2, v.u8());
        ch->setInfo(3, v.u8());
        ch->setByteLength(4 + 4 + 4); // chunk hdr + info hdr + 4 B info
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 4 + 4));
    }});

    // --- ABORT (type 6): fixed 4-byte chunk, no attached error causes (the
    // serializer has a "TODO handle attached error causes" and never writes any).
    fillers.push_back({"inet::sctp::SctpHeader", "abort", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAbortChunk("ABORT");
        ch->setSctpChunkType(6); // ABORT
        fillSctpChunkBase(ch, v);
        ch->setT_Bit(true);
        ch->setByteLength(4); // SCTP_ABORT_CHUNK_LENGTH
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4));
    }});

    // --- COOKIE_ECHO (type 10): the stateCookie-object path, NOT the raw cookie[]
    // bytes path -- SEE THE REPORT: COOKIE_ECHO's deserializer unconditionally
    // rebuilds a stateCookie object from a fixed-layout cookie_parameter struct
    // regardless of which of the two encodings produced the wire bytes, so the
    // cookie[] path can never come back the way it went out; the stateCookie path
    // below is the one that is actually symmetric. cookie[] and
    // unrecognizedParameters are left at default: see the report.
    fillers.push_back({"inet::sctp::SctpHeader", "cookieecho", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpCookieEchoChunk("COOKIE_ECHO");
        ch->setSctpChunkType(10); // COOKIE_ECHO
        fillSctpChunkBase(ch, v);
        ch->setStateCookie(makeSctpCookie(v));
        ch->setByteLength(4 + 76); // chunk hdr + cookie_parameter (4+4+4+32+32 B)
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 76));
    }});

    // --- COOKIE_ACK (type 11): fixed 4-byte chunk, no other fields.
    fillers.push_back({"inet::sctp::SctpHeader", "cookieack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpCookieAckChunk("COOKIE_ACK");
        ch->setSctpChunkType(11); // COOKIE_ACK
        fillSctpChunkBase(ch, v);
        ch->setByteLength(4); // SCTP_COOKIE_ACK_LENGTH
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4));
    }});

    // --- SHUTDOWN (type 7): fixed 8-byte chunk.
    fillers.push_back({"inet::sctp::SctpHeader", "shutdown", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpShutdownChunk("SHUTDOWN");
        ch->setSctpChunkType(7); // SHUTDOWN
        fillSctpChunkBase(ch, v);
        ch->setCumTsnAck(v.u32());
        ch->setByteLength(8); // SCTP_SHUTDOWN_CHUNK_LENGTH
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8));
    }});

    // --- SHUTDOWN_ACK (type 8): fixed 4-byte chunk, no other fields.
    fillers.push_back({"inet::sctp::SctpHeader", "shutdownack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpShutdownAckChunk("SHUTDOWN_ACK");
        ch->setSctpChunkType(8); // SHUTDOWN_ACK
        fillSctpChunkBase(ch, v);
        ch->setByteLength(4); // SCTP_SHUTDOWN_ACK_LENGTH
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4));
    }});

    // --- SHUTDOWN_COMPLETE (type 14): fixed 4-byte chunk, one flag bit.
    fillers.push_back({"inet::sctp::SctpHeader", "shutdowncomplete", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpShutdownCompleteChunk("SHUTDOWN_COMPLETE");
        ch->setSctpChunkType(14); // SHUTDOWN_COMPLETE
        fillSctpChunkBase(ch, v);
        ch->setTBit(true);
        ch->setByteLength(4);
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4));
    }});

    // --- ERROR (type 9): the empty-parameters branch (the only one that round-trips
    // -- see the report for why the two coded cause types, MISSING_NAT_ENTRY and
    // INVALID_STREAM_IDENTIFIER, cannot). TBit/MBit are real wire bits the serializer
    // writes, but the deserializer never reads flags back, so this variant is
    // expected to FAIL on those two fields (also flagged in the report).
    fillers.push_back({"inet::sctp::SctpHeader", "error", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpErrorChunk("ERROR");
        ch->setSctpChunkType(9); // ERRORTYPE
        fillSctpChunkBase(ch, v);
        ch->setTBit(true);
        ch->setMBit(false);
        ch->setByteLength(4); // SCTP_ERROR_CHUNK_LENGTH, no parameters
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4));
    }});

    // --- FORWARD_TSN (type 192): one stream entry (4 B, a multiple of 4).
    fillers.push_back({"inet::sctp::SctpHeader", "forwardtsn", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpForwardTsnChunk("FORWARD_TSN");
        ch->setSctpChunkType(192); // FORWARD_TSN
        fillSctpChunkBase(ch, v);
        ch->setNewCumTsn(v.u32());
        ch->setSidArraySize(1);
        ch->setSid(0, v.u16());
        ch->setSsnArraySize(1);
        ch->setSsn(0, (short)v.u16());
        ch->setByteLength(8 + 4); // SCTP_FORWARD_TSN_CHUNK_LENGTH + one stream entry
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 4));
    }});

    // --- AUTH (type 15): always exactly 8 + SHA_LENGTH(20) = 28 B on the wire,
    // regardless of the HMAC[] array's size -- the serializer always emits the fixed
    // 20-byte digest area (currently all zero: hmacSha1() is an unimplemented stub),
    // so HMAC[]/hMacOk are filled for coverage but do not affect the wire bytes.
    fillers.push_back({"inet::sctp::SctpHeader", "auth", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAuthenticationChunk("AUTH");
        ch->setSctpChunkType(15); // AUTH
        fillSctpChunkBase(ch, v);
        ch->setSharedKey(v.u16());
        ch->setHMacIdentifier(v.u16());
        ch->setHMacOk(true);
        ch->setHMACArraySize(2);
        ch->setHMAC(0, v.u32());
        ch->setHMAC(1, v.u32());
        ch->setByteLength(28); // SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH, fixed
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 28));
    }});

    // --- ASCONF (type 193): the mandatory leading IPv4 address parameter (8 B) plus
    // one ADD_IP_ADDRESS parameter (8 B header wrapping an 8 B nested IPv4 address).
    fillers.push_back({"inet::sctp::SctpHeader", "asconf", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfChunk("ASCONF");
        ch->setSctpChunkType(193); // ASCONF
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        ch->setAddressParam(v.l3Ipv4()); // mandatory leading address parameter: must be IPv4
        ch->setPeerVTag(v.u32());
        auto p = new SctpAddIPParameter("ADD_IP");
        p->setParameterType(49153); // ADD_IP_ADDRESS
        p->setRequestCorrelationId(v.u32());
        p->setAddressParam(v.l3Ipv4());
        p->setByteLength(16);
        ch->addAsconfParam(p); // @custom array: no setArraySize(), addAsconfParam() alone owns it
        ch->setByteLength(8 + 8 + 16); // chunk hdr+serial + mandatory address + one param
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 8 + 16));
    }});
    // the same with IPv6 addresses: an address parameter is 20 octets there, and both
    // directions have to pick the parameter type from the family
    fillers.push_back({"inet::sctp::SctpHeader", "asconf-v6", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfChunk("ASCONF");
        ch->setSctpChunkType(193); // ASCONF
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        ch->setAddressParam(v.l3Ipv6());
        ch->setPeerVTag(v.u32());
        auto p = new SctpAddIPParameter("ADD_IP");
        p->setParameterType(49153); // ADD_IP_ADDRESS
        p->setRequestCorrelationId(v.u32());
        p->setAddressParam(v.l3Ipv6());
        p->setByteLength(8 + 20);
        ch->addAsconfParam(p);
        ch->setByteLength(8 + 20 + 8 + 20);
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 20 + 8 + 20));
    }});
    // the same, but the trailing parameter is DELETE_IP_ADDRESS -- ADD_IP_ADDRESS's
    // sibling branch in asconf's per-parameter switch.
    fillers.push_back({"inet::sctp::SctpHeader", "asconf-delete", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfChunk("ASCONF");
        ch->setSctpChunkType(193); // ASCONF
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        ch->setAddressParam(v.l3Ipv4()); // mandatory leading address parameter: must be IPv4
        ch->setPeerVTag(v.u32());
        auto p = new SctpDeleteIPParameter("DELETE_IP");
        p->setParameterType(49154); // DELETE_IP_ADDRESS
        p->setRequestCorrelationId(v.u32());
        p->setAddressParam(v.l3Ipv4());
        p->setByteLength(16);
        ch->addAsconfParam(p);
        ch->setByteLength(8 + 8 + 16); // chunk hdr+serial + mandatory address + one param
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 8 + 16));
    }});
    // ... and SET_PRIMARY_ADDRESS, the third of the three sibling branches.
    fillers.push_back({"inet::sctp::SctpHeader", "asconf-setprimary", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfChunk("ASCONF");
        ch->setSctpChunkType(193); // ASCONF
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        ch->setAddressParam(v.l3Ipv4()); // mandatory leading address parameter: must be IPv4
        ch->setPeerVTag(v.u32());
        auto p = new SctpSetPrimaryIPParameter("SET_PRI_IP");
        p->setParameterType(49156); // SET_PRIMARY_ADDRESS
        p->setRequestCorrelationId(v.u32());
        p->setAddressParam(v.l3Ipv4());
        p->setByteLength(16);
        ch->addAsconfParam(p);
        ch->setByteLength(8 + 8 + 16); // chunk hdr+serial + mandatory address + one param
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 8 + 16));
    }});

    // --- ASCONF_ACK (type 128): one SUCCESS_INDICATION response parameter (8 B).
    fillers.push_back({"inet::sctp::SctpHeader", "asconfack", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfAckChunk("ASCONF_ACK");
        ch->setSctpChunkType(128); // ASCONF_ACK
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        auto p = new SctpSuccessIndication("SUCCESS");
        p->setParameterType(49157); // SUCCESS_INDICATION
        p->setResponseCorrelationId(v.u32());
        p->setByteLength(8);
        ch->addAsconfResponse(p); // @custom array: no setArraySize(), addAsconfResponse() alone owns it
        ch->setByteLength(8 + 8); // SCTP_ADD_IP_CHUNK_LENGTH + one response parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 8));
    }});
    // one ERROR_CAUSE_INDICATION response parameter for each of the three request
    // kinds it can echo back (ADD_IP_ADDRESS/DELETE_IP_ADDRESS/SET_PRIMARY_ADDRESS,
    // nested inside via encapsulate() -- the serializer reaches for
    // getEncapsulatedPacket()). SEE THE REPORT: ASCONF_ACK's deserializer rebuilds the
    // SctpErrorCauseParameter's type/correlation id/cause code but never restores its
    // encapsulated nested parameter, so on the second serialize
    // getEncapsulatedPacket() is nullptr; the guard around it,
    // `check_and_cast<SctpParameter *>(...) != nullptr`, assumes check_and_cast()
    // passes a null pointer through, but it throws on one instead -- so this variant
    // is expected to FAIL with a cRuntimeError ("Cannot cast nullptr to type
    // 'inet::sctp::SctpParameter *'") during re-serialize, not a plain byte diff.
    fillers.push_back({"inet::sctp::SctpHeader", "asconfack-error", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpAsconfAckChunk("ASCONF_ACK");
        ch->setSctpChunkType(128); // ASCONF_ACK
        fillSctpChunkBase(ch, v);
        ch->setSerialNumber(v.u32());
        // one ERROR_CAUSE_INDICATION per nested kind: 8 B (add_ip_parameter header)
        // + 4 B (error_cause header) + 16 B (nested ADD/DELETE/SET_PRIMARY parameter)
        {
            auto err = new SctpErrorCauseParameter("ERROR_CAUSE");
            err->setParameterType(49155); // ERROR_CAUSE_INDICATION
            err->setResponseCorrelationId(v.u32());
            err->setErrorCauseType(v.u32());
            auto nested = new SctpAddIPParameter("ADD_IP");
            nested->setParameterType(49153); // ADD_IP_ADDRESS
            nested->setRequestCorrelationId(v.u32());
            nested->setAddressParam(v.l3Ipv4());
            nested->setByteLength(16);
            err->encapsulate(nested);
            err->setByteLength(8 + 4 + 16); // after encapsulate(), which adds the nested length
            ch->addAsconfResponse(err);
        }
        {
            auto err = new SctpErrorCauseParameter("ERROR_CAUSE");
            err->setParameterType(49155); // ERROR_CAUSE_INDICATION
            err->setResponseCorrelationId(v.u32());
            err->setErrorCauseType(v.u32());
            auto nested = new SctpDeleteIPParameter("DELETE_IP");
            nested->setParameterType(49154); // DELETE_IP_ADDRESS
            nested->setRequestCorrelationId(v.u32());
            nested->setAddressParam(v.l3Ipv4());
            nested->setByteLength(16);
            err->encapsulate(nested);
            err->setByteLength(8 + 4 + 16); // after encapsulate(), which adds the nested length
            ch->addAsconfResponse(err);
        }
        {
            auto err = new SctpErrorCauseParameter("ERROR_CAUSE");
            err->setParameterType(49155); // ERROR_CAUSE_INDICATION
            err->setResponseCorrelationId(v.u32());
            err->setErrorCauseType(v.u32());
            auto nested = new SctpSetPrimaryIPParameter("SET_PRI_IP");
            nested->setParameterType(49156); // SET_PRIMARY_ADDRESS
            nested->setRequestCorrelationId(v.u32());
            nested->setAddressParam(v.l3Ipv4());
            nested->setByteLength(16);
            err->encapsulate(nested);
            err->setByteLength(8 + 4 + 16); // after encapsulate(), which adds the nested length
            ch->addAsconfResponse(err);
        }
        ch->setByteLength(8 + 3 * (8 + 4 + 16)); // SCTP_ADD_IP_CHUNK_LENGTH + three response parameters
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 8 + 3 * (8 + 4 + 16)));
    }});

    // --- RE_CONFIG / stream reset (type 130): one SSN/TSN reset request parameter
    // (the simplest of the five parameter kinds -- fixed 8 B, no trailing array).
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpSsnTsnResetRequestParameter("SSN_TSN_RST");
        p->setParameterType(15); // SSN_TSN_RESET_REQUEST_PARAMETER
        p->setSrReqSn(v.u32());
        p->setByteLength(8);
        ch->addParameter(p); // @custom array: no setArraySize(), addParameter() alone owns it
        ch->setByteLength(4 + 8); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 8));
    }});
    // the OUTGOING_RESET_REQUEST_PARAMETER branch, with an empty streamNumbers[] (a
    // non-empty one is exercised by neither this nor the "incoming" variant below --
    // see its report note -- so both are kept simple and streamNumbers-free).
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset-outgoing", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpOutgoingSsnResetRequestParameter("OUT_STR_RST");
        p->setParameterType(13); // OUTGOING_RESET_REQUEST_PARAMETER
        p->setSrReqSn(v.u32());
        p->setSrResSn(v.u32());
        p->setLastTsn(v.u32());
        p->setByteLength(16); // sizeof(outgoing_reset_request_parameter), no streamNumbers[]
        ch->addParameter(p);
        ch->setByteLength(4 + 16); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 16));
    }});
    // the INCOMING_RESET_REQUEST_PARAMETER branch. SEE THE REPORT: the deserializer's
    // streamNumbers-reading offset for this parameter is copy-pasted from the OUTGOING
    // branch above (reads 16 B past the parameter start instead of this parameter's own
    // 8 B header), so a non-empty streamNumbers[] would read garbage; an empty one
    // (the while loop's bound is never reached) sidesteps that entirely.
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset-incoming", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpIncomingSsnResetRequestParameter("IN_STR_RST");
        p->setParameterType(14); // INCOMING_RESET_REQUEST_PARAMETER
        p->setSrReqSn(v.u32());
        p->setByteLength(8); // sizeof(incoming_reset_request_parameter), no streamNumbers[]
        ch->addParameter(p);
        ch->setByteLength(4 + 8); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 8));
    }});
    // the STREAM_RESET_RESPONSE_PARAMETER branch, extended form: a non-zero
    // sendersNextTsn makes both the serializer and the deserializer carry the trailing
    // sendersNextTsn/receiversNextTsn pair, 20 B instead of the 12 B short form.
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset-response", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpStreamResetResponseParameter("STR_RST_RESP");
        p->setParameterType(16); // STREAM_RESET_RESPONSE_PARAMETER
        p->setSrResSn(v.u32());
        p->setResult(v.u32());
        p->setSendersNextTsn(v.u32()); // FillValues values are never 0: takes the extended form
        p->setReceiversNextTsn(v.u32());
        p->setByteLength(20); // extended stream_reset_response_parameter
        ch->addParameter(p);
        ch->setByteLength(4 + 20); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 20));
    }});
    // the ADD_INCOMING_STREAMS_REQUEST_PARAMETER branch. SEE THE REPORT: RE_CONFIG's
    // deserializer has no case for this parameter type at all (nor for its outgoing
    // counterpart below) -- neither is reconstructed, only skipped over via the
    // generic parptr advance -- so this variant is expected to FAIL the byte round
    // trip (the second serialize's parameter list is one entry short).
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset-addin", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpAddStreamsRequestParameter("ADD_IN_STR");
        p->setParameterType(18); // ADD_INCOMING_STREAMS_REQUEST_PARAMETER
        p->setSrReqSn(v.u32());
        p->setNumberOfStreams(v.u16());
        p->setByteLength(12); // sizeof(add_streams_request_parameter), fixed
        ch->addParameter(p);
        ch->setByteLength(4 + 12); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 12));
    }});
    // the ADD_OUTGOING_STREAMS_REQUEST_PARAMETER branch: same missing deserialize case
    // as ADD_INCOMING above, so also expected to FAIL the byte round trip.
    fillers.push_back({"inet::sctp::SctpHeader", "streamreset-addout", [](Chunk *c) {
        auto h = check_and_cast<SctpHeader *>(c);
        FillValues v;
        fillSctpHeaderBase(h, v);
        auto ch = new SctpStreamResetChunk("RE_CONFIG");
        ch->setSctpChunkType(130); // RE_CONFIG
        fillSctpChunkBase(ch, v);
        auto p = new SctpAddStreamsRequestParameter("ADD_OUT_STR");
        p->setParameterType(17); // ADD_OUTGOING_STREAMS_REQUEST_PARAMETER
        p->setSrReqSn(v.u32());
        p->setNumberOfStreams(v.u16());
        p->setByteLength(12); // sizeof(add_streams_request_parameter), fixed
        ch->addParameter(p);
        ch->setByteLength(4 + 12); // SCTP_STREAM_RESET_CHUNK_LENGTH + one parameter
        h->appendSctpChunks(ch);
        setChunkLength(c, B(12 + 4 + 12));
    }});

    // --- PKTDROP (type 129): SEE THE REPORT -- cannot be filled. The serializer
    // requires the chunk to encapsulate an inet::sctp::SctpHeader as its cPacket
    // payload (SctpHeaderSerializer.cc ~1087, "check_and_cast<SctpHeader *>(...
    // getEncapsulatedPacket())"), but SctpHeader is a Chunk, not a cPacket, and
    // cPacket::encapsulate() only accepts cMessage-derived objects -- there is no
    // way to construct a payload that check_and_cast would accept, and with none
    // encapsulated the cast throws immediately. The deserialize side is also fully
    // commented out (SctpHeaderSerializer.cc ~2131-2148: the whole case body is
    // `/* ... */; break;`), so even a hypothetical valid PKTDROP could never be
    // reconstructed. No filler is provided.
}

} // namespace inet
