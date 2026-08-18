//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the connection-oriented/-less transport headers: UDP, and TCP
// with one variant per header-option type.
//

#include "../ChunkFillers.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

namespace {

// The fixed part of a TcpHeader: every scalar field distinct, and every flag bit set
// to true (the .msg default is false for all eight, so this variant alone covers all
// of them; the option variants below re-derive their own headerLength/chunkLength on
// top of this). commonSetup already set checksumMode to CHECKSUM_COMPUTED.
void fillTcpBase(tcp::TcpHeader *h, FillValues& v)
{
    h->setSrcPort(v.u16());
    h->setDestPort(v.u16());
    h->setSequenceNo(v.u32());
    h->setAckNo(v.u32());
    h->setCwrBit(true);
    h->setEceBit(true);
    h->setUrgBit(true);
    h->setAckBit(true);
    h->setPshBit(true);
    h->setRstBit(true);
    h->setSynBit(true);
    h->setFinBit(true);
    h->setWindow(v.u16());
    h->setUrgentPointer(v.u16());
    h->setChecksum(v.u16());
}

// Append one header option (plus, if needed, TcpOptionNop padding to the next 32-bit
// word -- the serializer asserts headerLength == 20 + sum(option lengths) and that the
// total is a whole number of words) and set headerLength/chunkLength to match.
void fillTcpOption(Chunk *c, tcp::TcpOption *opt)
{
    auto h = check_and_cast<tcp::TcpHeader *>(c);
    h->appendHeaderOption(opt);
    int optLen = opt->getLength();
    while (optLen % 4 != 0) {
        h->appendHeaderOption(new tcp::TcpOptionNop());
        optLen += 1;
    }
    B len = tcp::TCP_MIN_HEADER_LENGTH + B(optLen);
    h->setHeaderLength(len);
    setChunkLength(c, len);
}

} // namespace

void addFillers_transport(std::vector<ChunkFiller>& fillers)
{
    // --- UDP: a fixed-size header, so chunkLength keeps its .msg default (8 B).
    // totalLengthField (header + payload) has no other constraint the serializer
    // checks, so any value round-trips; give it a real one rather than the -1
    // "unset" sentinel.
    fillers.push_back({"inet::UdpHeader", "", [](Chunk *c) {
        auto h = check_and_cast<UdpHeader *>(c);
        FillValues v;
        h->setSrcPort(v.u16());
        h->setDestPort(v.u16());
        h->setTotalLengthField(B(v.u16()));
        h->setChecksum(v.u16());
    }});

    // --- TCP: the plain 20-byte header (all flag bits, see fillTcpBase), then one
    // variant per header-option type so each of the serializer's option branches
    // (RFC 793 EOL/NOP, RFC 793 MSS, RFC 1323 WS/TS, RFC 2018 SACK-permitted/SACK,
    // plus the generic/unrecognized-kind fallback) is exercised.
    fillers.push_back({"inet::tcp::TcpHeader", "", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        h->setHeaderLength(tcp::TCP_MIN_HEADER_LENGTH);
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "eol", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        fillTcpOption(c, new tcp::TcpOptionEnd());
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "nop", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        fillTcpOption(c, new tcp::TcpOptionNop());
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "mss", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        auto o = new tcp::TcpOptionMaxSegmentSize();
        o->setMaxSegmentSize(v.u16());
        fillTcpOption(c, o);
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "wscale", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        auto o = new tcp::TcpOptionWindowScale();
        o->setWindowScale(v.uint(8));
        fillTcpOption(c, o);
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "sack-permitted", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        fillTcpOption(c, new tcp::TcpOptionSackPermitted());
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "timestamp", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        auto o = new tcp::TcpOptionTimestamp();
        o->setSenderTimestamp(v.u32());
        o->setEchoedTimestamp(v.u32());
        fillTcpOption(c, o);
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "sack", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        auto o = new tcp::TcpOptionSack();
        o->setSackItemArraySize(2);
        tcp::SackItem si0;
        si0.setStart(v.u32());
        si0.setEnd(v.u32());
        o->setSackItem(0, si0);
        tcp::SackItem si1;
        si1.setStart(v.u32());
        si1.setEnd(v.u32());
        o->setSackItem(1, si1);
        o->setLength(2 + 2 * 8);
        fillTcpOption(c, o);
    }});
    fillers.push_back({"inet::tcp::TcpHeader", "unknown", [](Chunk *c) {
        auto h = check_and_cast<tcp::TcpHeader *>(c);
        FillValues v;
        fillTcpBase(h, v);
        auto o = new tcp::TcpOptionUnknown();
        o->setKind(static_cast<tcp::TcpOptionNumbers>(28)); // unassigned per the .msg's IANA table
        o->setBytesArraySize(2);
        o->setBytes(0, v.u8());
        o->setBytes(1, v.u8());
        o->setLength(2 + 2);
        fillTcpOption(c, o);
    }});
}

} // namespace inet
