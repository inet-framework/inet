//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the IPv4 network layer: ARP, the IPv4 header (and one case per
// header-option type), ICMP and IGMP.
//

#include "../ChunkFillers.h"
#include "inet/common/Protocol.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

namespace {

// The IPv4 header without options: a header-only packet, so the total length is the
// header length. The fragment offset is a 13-bit field in units of 8 octets, and the
// payload protocol serializes as the 1-byte protocolId through getIpProtocolGroup(),
// so it must name a protocol registered in that group.
void fillIpv4HeaderBase(Ipv4Header *h, FillValues& v)
{
    h->setVersion(4);
    h->setTypeOfService(v.u8());
    h->setIdentification(v.u16());
    h->setReservedBit(true);
    h->setMoreFragments(true);
    h->setDontFragment(false);
    h->setFragmentOffset(8 * v.uint(10));
    h->setTimeToLive(v.u8());
    h->setProtocol(&Protocol::udp);
    h->setChecksum(v.u16());
    h->setChecksumMode(CHECKSUM_COMPUTED);
    h->setSrcAddress(v.ipv4());
    h->setDestAddress(v.ipv4());
}

// The IPv4 header carrying one option: the header length grows to a whole number of
// 32-bit words (the serializer pads the remainder with End options, which round-trip),
// and the total length follows it -- headerLength <= totalLengthField is asserted.
void fillIpv4HeaderWithOption(Chunk *c, TlvOptionBase *opt)
{
    auto h = check_and_cast<Ipv4Header *>(c);
    FillValues v;
    fillIpv4HeaderBase(h, v);
    h->addOption(opt);
    B len = IPv4_MIN_HEADER_LENGTH + B(opt->getLength());
    if (len.get<B>() % 4 != 0)
        len += B(4 - len.get<B>() % 4);
    h->setHeaderLength(len);
    h->setTotalLengthField(len);
    setChunkLength(c, len);
}

void fillIgmpBase(IgmpMessage *p, FillValues& v)
{
    p->setChecksum(v.u16());
    p->setChecksumMode(CHECKSUM_COMPUTED);
}

} // namespace

void addFillers_ipv4(std::vector<ChunkFiller>& fillers)
{
    fillers.push_back({"inet::ArpPacket", "", [](Chunk *c) {
        auto p = check_and_cast<ArpPacket *>(c);
        FillValues v;
        p->setOpcode(ARP_REQUEST); // 0 is not a valid opcode
        p->setSrcMacAddress(v.mac());
        p->setDestMacAddress(v.mac());
        p->setSrcIpAddress(v.ipv4());
        p->setDestIpAddress(v.ipv4());
    }});

    // --- IPv4 header: no options, then one case per option type so each option's own
    // serializer branch is exercised (they share the header serializer's option loop).
    fillers.push_back({"inet::Ipv4Header", "", [](Chunk *c) {
        auto h = check_and_cast<Ipv4Header *>(c);
        FillValues v;
        fillIpv4HeaderBase(h, v);
        h->setHeaderLength(IPv4_MIN_HEADER_LENGTH);
        h->setTotalLengthField(IPv4_MIN_HEADER_LENGTH); // header-only packet
    }});
    fillers.push_back({"inet::Ipv4Header", "streamid", [](Chunk *c) {
        FillValues v;
        auto o = new Ipv4OptionStreamId();
        o->setStreamId(v.u16());
        fillIpv4HeaderWithOption(c, o);
    }});
    fillers.push_back({"inet::Ipv4Header", "routeralert", [](Chunk *c) {
        auto o = new Ipv4OptionRouterAlert();
        o->setRouterAlert(0); // the only value RFC 2113 defines
        fillIpv4HeaderWithOption(c, o);
    }});
    fillers.push_back({"inet::Ipv4Header", "timestamp", [](Chunk *c) {
        FillValues v;
        auto o = new Ipv4OptionTimestamp();
        o->setFlag(IP_TIMESTAMP_TIMESTAMP_ONLY);
        o->setOverflow(v.uint(4));
        o->setRecordTimestampArraySize(1);
        o->setRecordTimestamp(0, v.time());
        o->setNextIdx(1);
        o->setLength(4 + 4);
        fillIpv4HeaderWithOption(c, o);
    }});
    fillers.push_back({"inet::Ipv4Header", "recordroute", [](Chunk *c) {
        FillValues v;
        auto o = new Ipv4OptionRecordRoute();
        o->setType(IPOPTION_RECORD_ROUTE); // shared with the two source-routing options
        o->setRecordAddressArraySize(1);
        o->setRecordAddress(0, v.ipv4());
        o->setNextAddressIdx(1);
        o->setLength(3 + 4 * 1);
        fillIpv4HeaderWithOption(c, o);
    }});
    // the No-Operation option is a single octet with no length field of its own
    fillers.push_back({"inet::Ipv4Header", "nop", [](Chunk *c) {
        auto o = new Ipv4OptionNop();
        o->setType(IPOPTION_NO_OPTION);
        o->setLength(1);
        fillIpv4HeaderWithOption(c, o);
    }});
    // the timestamp option's two address-carrying flag values: each record is an
    // address and a timestamp, so a record is 8 octets instead of 4
    for (auto flag : { IP_TIMESTAMP_WITH_ADDRESS, IP_TIMESTAMP_SENDER_INIT_ADDRESS }) {
        fillers.push_back({"inet::Ipv4Header", flag == IP_TIMESTAMP_WITH_ADDRESS ? "timestamp-addr" : "timestamp-senderinit", [flag](Chunk *c) {
            FillValues v;
            auto o = new Ipv4OptionTimestamp();
            o->setFlag(flag);
            o->setOverflow(v.uint(4));
            o->setRecordAddressArraySize(1);
            o->setRecordAddress(0, v.ipv4());
            o->setRecordTimestampArraySize(1);
            o->setRecordTimestamp(0, v.time());
            o->setNextIdx(1);
            o->setLength(4 + 8);
            fillIpv4HeaderWithOption(c, o);
        }});
    }

    // any option type INET does not model travels as raw bytes; the Security option has
    // its own (equally raw) branch on the read side
    for (int type : { 30, (int)IPOPTION_SECURITY }) {
        fillers.push_back({"inet::Ipv4Header", type == 30 ? "raw" : "security", [type](Chunk *c) {
        FillValues v;
        auto o = new TlvOptionRaw();
        o->setType(type);
        o->setBytesArraySize(2);
        o->setBytes(0, v.u8());
        o->setBytes(1, v.u8());
        o->setLength(2 + 2);
        fillIpv4HeaderWithOption(c, o);
        // the unfragmented, don't-fragment combination: with the other variants' more-
        // fragments state this covers both flag bits in both values
        auto h = check_and_cast<Ipv4Header *>(c);
        h->setMoreFragments(false);
        h->setDontFragment(true);
        h->setFragmentOffset(0);
        }});
    }

    // --- ICMP. The type is pinned by the subclass, the code by the message kind.
    fillers.push_back({"inet::IcmpEchoRequest", "", [](Chunk *c) {
        auto p = check_and_cast<IcmpEchoRequest *>(c);
        FillValues v;
        p->setCode(0);
        p->setChksum(v.u16());
        p->setChecksumMode(CHECKSUM_COMPUTED);
        p->setIdentifier(v.u16());
        p->setSeqNumber(v.u16());
    }});
    fillers.push_back({"inet::IcmpEchoReply", "", [](Chunk *c) {
        auto p = check_and_cast<IcmpEchoReply *>(c);
        FillValues v;
        p->setCode(0);
        p->setChksum(v.u16());
        p->setChecksumMode(CHECKSUM_COMPUTED);
        p->setIdentifier(v.u16());
        p->setSeqNumber(v.u16());
    }});
    fillers.push_back({"inet::IcmpPtb", "", [](Chunk *c) {
        auto p = check_and_cast<IcmpPtb *>(c);
        FillValues v;
        p->setCode(ICMP_DU_FRAGMENTATION_NEEDED);
        p->setChksum(v.u16());
        p->setChecksumMode(CHECKSUM_COMPUTED);
        p->setUnused(0); // RFC 792: sent as zero
        p->setMtu(v.u16());
    }});

    // --- IGMP. v1 and v2 queries differ only in the second header byte (v1 sends it
    // as unused, v2 as the max-response code), so both are their own case.
    fillers.push_back({"inet::Igmpv1Query", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv1Query *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setUnused(0); // RFC 1112: sent as zero
        p->setGroupAddress(v.ipv4());
    }});
    fillers.push_back({"inet::Igmpv1Report", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv1Report *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setUnused(0);
        p->setGroupAddress(v.ipv4());
    }});
    fillers.push_back({"inet::Igmpv2Query", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv2Query *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setMaxRespTimeCode(v.u8());
        p->setGroupAddress(v.ipv4());
    }});
    fillers.push_back({"inet::Igmpv2Report", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv2Report *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setMaxRespTime(v.u8());
        p->setGroupAddress(v.ipv4());
    }});
    fillers.push_back({"inet::Igmpv2Leave", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv2Leave *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setMaxRespTime(v.u8());
        p->setGroupAddress(v.ipv4());
    }});
    // v3 query: a general query (no sources) and a source-specific one, so both the
    // empty and the populated source-list branch of the serializer run. The source
    // list is an Ipv4AddressVector (a bare std::vector with no class descriptor), so
    // the coverage audit cannot see its contents -- hence the knownUnfilled entry.
    for (int sources : {0, 3}) {
        fillers.push_back({"inet::Igmpv3Query", sources == 0 ? "general" : "sources", [sources](Chunk *c) {
            auto p = check_and_cast<Igmpv3Query *>(c);
            FillValues v;
            fillIgmpBase(p, v);
            p->setMaxRespTimeCode(v.u8());
            p->setGroupAddress(v.ipv4());
            p->setResv(v.uint(4));
            p->setSuppressRouterProc(true);
            p->setRobustnessVariable(v.uint(3));
            p->setQueryIntervalCode(v.u8());
            for (int i = 0; i < sources; i++)
                p->getSourceListForUpdate().push_back(v.ipv4());
            setChunkLength(c, B(12) + B(4 * sources));
        }});
    }
    // RGMP (Cisco Router-port Group Management Protocol) rides on the IGMP serializer
    fillers.push_back({"inet::RgmpHello", "", [](Chunk *c) {
        auto p = check_and_cast<RgmpHello *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setReserved(0); // sent as zero
        p->setGroupAddress(v.ipv4());
    }});
    // v3 report: one record per record type layout -- an empty one, one with sources,
    // and one with auxiliary data (the three loops the serializer walks per record).
    fillers.push_back({"inet::Igmpv3Report", "", [](Chunk *c) {
        auto p = check_and_cast<Igmpv3Report *>(c);
        FillValues v;
        fillIgmpBase(p, v);
        p->setResv1(0);
        p->setResv2(0);
        p->setGroupRecordArraySize(3);
        GroupRecord r0;
        r0.setRecordType(MODE_IS_INCLUDE);
        r0.setGroupAddress(v.ipv4());
        p->setGroupRecord(0, r0);
        GroupRecord r1;
        r1.setRecordType(CHANGE_TO_EXCLUDE_MODE);
        r1.setGroupAddress(v.ipv4());
        r1.getSourceListForUpdate().push_back(v.ipv4());
        r1.getSourceListForUpdate().push_back(v.ipv4());
        p->setGroupRecord(1, r1);
        GroupRecord r2;
        r2.setRecordType(BLOCK_OLD_SOURCE);
        r2.setGroupAddress(v.ipv4());
        r2.setAuxDataArraySize(2);
        r2.setAuxData(0, v.u32());
        r2.setAuxData(1, v.u32());
        p->setGroupRecord(2, r2);
        measureAndSetChunkLength(c);
    }});
}

} // namespace inet
