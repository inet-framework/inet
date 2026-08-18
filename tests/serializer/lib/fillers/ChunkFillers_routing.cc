//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the routing-protocol control packets: AODV, BGPv4, PIM, RIP, DSDV.
//

#include "../ChunkFillers.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/routing/dsdv/DsdvHello_m.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/rip/RipPacket_m.h"

namespace inet {

namespace {

// ===================== AODV =====================

// AODV carries its packetType discriminator in the base class's field, whose default is
// an out-of-range sentinel and which the msg compiler marks non-editable; set it (and the
// variant's fixed wire length, from the .msg comments) with the typed C++ setters.
void setAodvBase(aodv::AodvControlPacket *p, aodv::AodvControlPacketType type, B length)
{
    p->setPacketType(type);
    setChunkLength(p, length);
}

// ===================== BGP =====================

void fillBgpOpenBase(bgp::BgpOpenMessage *p, FillValues& v)
{
    p->setVersion(4); // the only version BgpHeaderSerializer accepts
    p->setMyAS(v.u16());
    p->setHoldTime(SimTime(3 + v.u8(), SIMTIME_S)); // must be 0 or >= 3 (else the serializer throws)
    p->setBgpIdentifier(v.ipv4());
}

// withdrawn-route / plain-NLRI prefixes are written as a 1-byte length followed by
// ceil(length/8) prefix bytes (BgpHeaderSerializer.cc); using byte-aligned lengths avoids
// the bit-padding arithmetic.
int prefixWireBytes(int bits)
{
    return 1 + (bits + 7) / 8;
}

// ===================== PIM =====================

void fillPimHeader(PimPacket *p, FillValues& v)
{
    p->setVersion(2); // the only version PimPacketSerializer implements
    p->setReserved(v.uint(8)); // regular data field: fully round-tripped, no fixed value required
    p->setChecksum(v.u16());
    // checksumMode is set by commonSetup
}

EncodedUnicastAddress mkUnicast(FillValues& v, bool ipv6)
{
    EncodedUnicastAddress a;
    a.unicastAddress = ipv6 ? v.l3Ipv6() : v.l3Ipv4();
    return a;
}

// maskLength is validated by deserializeEncodedGroupAddress/-SourceAddress (32 for IPv4,
// 128 for IPv6 native encoding); the other fields fully round-trip regardless of value.
EncodedGroupAddress mkGroup(FillValues& v, bool ipv6)
{
    EncodedGroupAddress g;
    g.B = v.flag();
    g.reserved = v.uint(6);
    g.Z = v.flag();
    g.maskLength = ipv6 ? 128 : 32;
    g.groupAddress = ipv6 ? v.l3Ipv6() : v.l3Ipv4();
    return g;
}

EncodedSourceAddress mkSource(FillValues& v, bool ipv6)
{
    EncodedSourceAddress s;
    s.reserved = v.uint(5);
    s.S = v.flag();
    s.W = v.flag();
    s.R = v.flag();
    s.maskLength = ipv6 ? 128 : 32;
    s.sourceAddress = ipv6 ? v.l3Ipv6() : v.l3Ipv4();
    return s;
}

JoinPruneGroup mkJoinPruneGroup(FillValues& v, bool ipv6)
{
    JoinPruneGroup g;
    g.setGroupAddress(mkGroup(v, ipv6));
    g.setJoinedSourceAddressArraySize(1);
    g.setJoinedSourceAddress(0, mkSource(v, ipv6));
    g.setPrunedSourceAddressArraySize(1);
    g.setPrunedSourceAddress(0, mkSource(v, ipv6));
    return g;
}

// Shared body for PimJoinPrune and its PimGraft subclass. For Graft/GraftAck the wire
// holdTime is hardcoded to 0 by the serializer regardless of the field's value (and the
// .msg pins PimGraft's own default to 0), so a "fixed zero" caller leaves it untouched.
void fillJoinPruneBody(PimJoinPrune *p, FillValues& v, bool ipv6, bool holdTimeIsFixedZero)
{
    p->setUpstreamNeighborAddress(mkUnicast(v, ipv6));
    p->setReserved2(v.u8());
    if (!holdTimeIsFixedZero)
        p->setHoldTime(v.u16());
    p->setJoinPruneGroupsArraySize(1);
    p->setJoinPruneGroups(0, mkJoinPruneGroup(v, ipv6));
}

} // namespace

void addFillers_routing(std::vector<ChunkFiller>& fillers)
{
    // ===================== AODV =====================
    // packetType carries an IPv4 and an IPv6 form per message (RREQ vs RREQ_IPv6, ...) with
    // different address widths and a different field order on the wire; each gets its own
    // filler so both serializer branches run.
    fillers.push_back({"inet::aodv::Rreq", "RREQ", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rreq *>(c);
        FillValues v;
        p->setJoinFlag(v.flag());
        p->setRepairFlag(v.flag());
        p->setGratuitousRREPFlag(v.flag());
        p->setDestOnlyFlag(v.flag());
        p->setUnknownSeqNumFlag(v.flag());
        p->setReserved(0); // .msg: "Sent as 0; ignored on reception."
        p->setHopCount(v.u8());
        p->setRreqId(v.u32());
        p->setDestAddr(v.l3Ipv4());
        p->setDestSeqNum(v.u32());
        p->setOriginatorAddr(v.l3Ipv4());
        p->setOriginatorSeqNum(v.u32());
        setAodvBase(p, aodv::RREQ, B(24));
    }});
    fillers.push_back({"inet::aodv::Rreq", "RREQ_IPv6", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rreq *>(c);
        FillValues v;
        p->setJoinFlag(v.flag());
        p->setRepairFlag(v.flag());
        p->setGratuitousRREPFlag(v.flag());
        p->setDestOnlyFlag(v.flag());
        p->setUnknownSeqNumFlag(v.flag());
        p->setReserved(0);
        p->setHopCount(v.u8());
        p->setRreqId(v.u32());
        p->setDestAddr(v.l3Ipv6());
        p->setDestSeqNum(v.u32());
        p->setOriginatorAddr(v.l3Ipv6());
        p->setOriginatorSeqNum(v.u32());
        setAodvBase(p, aodv::RREQ_IPv6, B(48));
    }});
    fillers.push_back({"inet::aodv::Rrep", "RREP", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rrep *>(c);
        FillValues v;
        p->setRepairFlag(v.flag());
        p->setAckRequiredFlag(v.flag());
        p->setReserved(0); // .msg: "Sent as 0; ignored on reception."
        p->setPrefixSize(v.uint(5)); // 5-bit field on the IPv4 wire form
        p->setHopCount(v.u8());
        p->setDestAddr(v.l3Ipv4());
        p->setDestSeqNum(v.u32());
        p->setOriginatorAddr(v.l3Ipv4());
        p->setLifeTime(v.time());
        setAodvBase(p, aodv::RREP, B(20));
    }});
    fillers.push_back({"inet::aodv::Rrep", "RREP_IPv6", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rrep *>(c);
        FillValues v;
        p->setRepairFlag(v.flag());
        p->setAckRequiredFlag(v.flag());
        p->setReserved(0);
        p->setPrefixSize(v.uint(7)); // 7-bit field on the IPv6 wire form
        p->setHopCount(v.u8());
        p->setDestAddr(v.l3Ipv6());
        p->setDestSeqNum(v.u32());
        p->setOriginatorAddr(v.l3Ipv6());
        p->setLifeTime(v.time());
        setAodvBase(p, aodv::RREP_IPv6, B(44));
    }});
    // Rerr needs at least one unreachable node (the serializer throws on an empty list);
    // use two so the reverse-order write/read loop is exercised too.
    fillers.push_back({"inet::aodv::Rerr", "RERR", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rerr *>(c);
        FillValues v;
        p->setNoDeleteFlag(v.flag());
        p->setReserved(0); // .msg: "Sent as 0; ignored on reception."
        p->setUnreachableNodesArraySize(2);
        for (int i = 0; i < 2; i++) {
            aodv::UnreachableNode n;
            n.addr = v.l3Ipv4();
            n.seqNum = v.u32();
            p->setUnreachableNodes(i, n);
        }
        setAodvBase(p, aodv::RERR, B(4 + 2 * 8));
    }});
    fillers.push_back({"inet::aodv::Rerr", "RERR_IPv6", [](Chunk *c) {
        auto p = check_and_cast<aodv::Rerr *>(c);
        FillValues v;
        p->setNoDeleteFlag(v.flag());
        p->setReserved(0);
        p->setUnreachableNodesArraySize(2);
        for (int i = 0; i < 2; i++) {
            aodv::UnreachableNode n;
            n.addr = v.l3Ipv6();
            n.seqNum = v.u32();
            p->setUnreachableNodes(i, n);
        }
        setAodvBase(p, aodv::RERR_IPv6, B(4 + 2 * (4 + 16)));
    }});
    fillers.push_back({"inet::aodv::RrepAck", "RREPACK", [](Chunk *c) {
        auto p = check_and_cast<aodv::RrepAck *>(c);
        FillValues v;
        p->setReserved(v.u8()); // no ".msg: sent as 0" note here, and the serializer round-trips it verbatim
        setAodvBase(p, aodv::RREPACK, B(2));
    }});
    fillers.push_back({"inet::aodv::RrepAck", "RREPACK_IPv6", [](Chunk *c) {
        auto p = check_and_cast<aodv::RrepAck *>(c);
        FillValues v;
        p->setReserved(v.u8());
        setAodvBase(p, aodv::RREPACK_IPv6, B(2));
    }});

    // ===================== BGP =====================
    // KEEPALIVE carries nothing beyond the (already .msg-defaulted) BgpHeader: marker
    // (0xFF x16), totalLength (19) and chunkLength (19) are all already meaningful defaults.
    fillers.push_back({"inet::bgp::BgpKeepAliveMessage", "", [](Chunk *) {
    }});

    // OPEN: the optional-parameter list has two shapes -- a raw parameter and the RFC 5492
    // Capabilities parameter (here an RFC 4760 Multiprotocol capability, AFI 1 and AFI 2).
    fillers.push_back({"inet::bgp::BgpOpenMessage", "raw", [](Chunk *c) {
        auto p = check_and_cast<bgp::BgpOpenMessage *>(c);
        FillValues v;
        fillBgpOpenBase(p, v);
        auto *op = new bgp::BgpOptionalParameterRaw();
        op->setParameterType(1); // any type other than 2 (Capabilities) selects the raw branch
        op->setValueArraySize(2);
        op->setValue(0, (char)v.u8());
        op->setValue(1, (char)v.u8());
        op->setParameterValueLength(2);
        p->setOptionalParameterArraySize(1);
        p->setOptionalParameter(0, op);
        p->setOptionalParametersLength((uint16_t)(2 + op->getParameterValueLength()));
        p->setTotalLength((uint16_t)((bgp::BGP_HEADER_OCTETS + bgp::BGP_OPEN_OCTETS).get<B>()
                + p->getOptionalParametersLength()));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::bgp::BgpOpenMessage", "capabilities-ipv4", [](Chunk *c) {
        auto p = check_and_cast<bgp::BgpOpenMessage *>(c);
        FillValues v;
        fillBgpOpenBase(p, v);
        auto *mp = new bgp::BgpCapabilityMultiprotocol();
        mp->setAfi(1); // IPv4
        mp->setReserved(0); // .msg: "1 octet, sent as 0"
        mp->setSafi(1); // unicast
        auto *caps = new bgp::BgpOptionalParameterCapabilities();
        caps->setCapabilityArraySize(1);
        caps->setCapability(0, mp);
        caps->setParameterValueLength((uint16_t)(2 + mp->getCapabilityLength()));
        p->setOptionalParameterArraySize(1);
        p->setOptionalParameter(0, caps);
        p->setOptionalParametersLength((uint16_t)(2 + caps->getParameterValueLength()));
        p->setTotalLength((uint16_t)((bgp::BGP_HEADER_OCTETS + bgp::BGP_OPEN_OCTETS).get<B>()
                + p->getOptionalParametersLength()));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::bgp::BgpOpenMessage", "capabilities-ipv6", [](Chunk *c) {
        auto p = check_and_cast<bgp::BgpOpenMessage *>(c);
        FillValues v;
        fillBgpOpenBase(p, v);
        auto *mp = new bgp::BgpCapabilityMultiprotocol();
        mp->setAfi(2); // IPv6
        mp->setReserved(0);
        mp->setSafi(1);
        auto *caps = new bgp::BgpOptionalParameterCapabilities();
        caps->setCapabilityArraySize(1);
        caps->setCapability(0, mp);
        caps->setParameterValueLength((uint16_t)(2 + mp->getCapabilityLength()));
        p->setOptionalParameterArraySize(1);
        p->setOptionalParameter(0, caps);
        p->setOptionalParametersLength((uint16_t)(2 + caps->getParameterValueLength()));
        p->setTotalLength((uint16_t)((bgp::BGP_HEADER_OCTETS + bgp::BGP_OPEN_OCTETS).get<B>()
                + p->getOptionalParametersLength()));
        measureAndSetChunkLength(c);
    }});

    // UPDATE: withdrawn routes, NLRI, and the path-attribute list. This variant covers the
    // IPv4-style attributes (ORIGIN/AS_PATH/NEXT_HOP/MULTI_EXIT_DISC/LOCAL_PREF/
    // ATOMIC_AGGREGATE/AGGREGATOR) plus BgpUpdatePathAttributesUnknown, the fallback for an
    // unmodelled attribute type (here COMMUNITIES, RFC 1997); the RFC 4760 Multiprotocol
    // attributes get their own "mp-ipv6" variant below.
    fillers.push_back({"inet::bgp::BgpUpdateMessage", "", [](Chunk *c) {
        auto p = check_and_cast<bgp::BgpUpdateMessage *>(c);
        FillValues v;

        bgp::BgpUpdateWithdrawnRoutes wr0; wr0.length = 24; wr0.prefix = v.ipv4();
        bgp::BgpUpdateWithdrawnRoutes wr1; wr1.length = 16; wr1.prefix = v.ipv4();
        p->setWithdrawnRoutesArraySize(2);
        p->setWithdrawnRoutes(0, wr0);
        p->setWithdrawnRoutes(1, wr1);
        p->setWithDrawnRoutesLength((uint16_t)(prefixWireBytes(24) + prefixWireBytes(16)));

        std::vector<bgp::BgpUpdatePathAttributes *> attrs;
        unsigned totalAttrLen = 0;

        // optionalBit/transitiveBit are pinned per attribute type by the .msg (RFC 4271);
        // any other combination trips the flags-consistency throw in
        // BgpHeaderSerializer::serialize, so they are left at that pinned value below.
        // The 4-bit 'reserved' flags field is left at its default 0 for the same reason:
        // BgpHeaderSerializer::deserialize requires it to read back as zero (readBitRepeatedly)
        // and unconditionally reconstructs it as 0, so a non-zero value would desync the
        // second serialize pass from the first.

        auto *origin = new bgp::BgpUpdatePathAttributesOrigin();
        origin->setPartialBit(false); // well-known attribute: the flags check requires this
        origin->setExtendedLengthBit(true); // exercise the 2-byte length-field branch
        origin->setValue(bgp::INCOMPLETE);
        totalAttrLen += 4 + 1; // header(flags+reserved, typeCode, 2-byte length) + 1-byte value
        attrs.push_back(origin);

        auto *asPath = new bgp::BgpUpdatePathAttributesAsPath();
        asPath->setPartialBit(false);
        asPath->setExtendedLengthBit(false);
        bgp::BgpAsPathSegment seg;
        seg.setType(bgp::AS_SEQUENCE);
        seg.setLength(2);
        seg.setAsValueArraySize(2);
        seg.setAsValue(0, v.u16());
        seg.setAsValue(1, v.u16());
        asPath->setValueArraySize(1);
        asPath->setValue(0, seg);
        asPath->setLength(2 + 2 * 2); // segment header(type+count) + 2 AS numbers; .msg default (0) is wrong here
        totalAttrLen += 3 + asPath->getLength();
        attrs.push_back(asPath);

        auto *nextHop = new bgp::BgpUpdatePathAttributesNextHop();
        nextHop->setPartialBit(false);
        nextHop->setExtendedLengthBit(false);
        nextHop->setValue(v.ipv4());
        totalAttrLen += 3 + 4;
        attrs.push_back(nextHop);

        auto *med = new bgp::BgpUpdatePathAttributesMultiExitDisc();
        med->setPartialBit(false); // optional non-transitive: the flags check requires this too
        med->setExtendedLengthBit(false);
        med->setValue(v.u32());
        totalAttrLen += 3 + 4;
        attrs.push_back(med);

        auto *localPref = new bgp::BgpUpdatePathAttributesLocalPref();
        localPref->setPartialBit(false);
        localPref->setExtendedLengthBit(false);
        localPref->setValue(v.u32());
        totalAttrLen += 3 + 4;
        attrs.push_back(localPref);

        auto *atomicAggregate = new bgp::BgpUpdatePathAttributesAtomicAggregate();
        atomicAggregate->setPartialBit(false);
        atomicAggregate->setExtendedLengthBit(false);
        atomicAggregate->setLength(0); // fixed: ATOMIC_AGGREGATE carries no value
        totalAttrLen += 3 + 0;
        attrs.push_back(atomicAggregate);

        // AGGREGATOR is the one type whose flags check tolerates partialBit == true
        // (optionalBit=true, transitiveBit=true): use it to exercise that branch too.
        auto *aggregator = new bgp::BgpUpdatePathAttributesAggregator();
        aggregator->setPartialBit(true);
        aggregator->setExtendedLengthBit(false);
        aggregator->setAsNumber(v.u16());
        aggregator->setBgpSpeaker(v.ipv4());
        totalAttrLen += 3 + 6;
        attrs.push_back(aggregator);

        auto *unknown = new bgp::BgpUpdatePathAttributesUnknown();
        unknown->setOptionalBit(true);
        unknown->setTransitiveBit(true);
        unknown->setPartialBit(false);
        unknown->setExtendedLengthBit(false);
        unknown->setTypeCode(static_cast<bgp::BgpUpdateAttributeTypeCode>(8)); // COMMUNITIES (RFC 1997): not modelled
        unknown->setValueArraySize(3);
        unknown->setValue(0, v.u8());
        unknown->setValue(1, v.u8());
        unknown->setValue(2, v.u8());
        unknown->setLength(3);
        totalAttrLen += 3 + 3;
        attrs.push_back(unknown);

        p->setPathAttributesArraySize(attrs.size());
        for (size_t i = 0; i < attrs.size(); i++)
            p->setPathAttributes(i, attrs[i]);
        p->setTotalPathAttributeLength((uint16_t)totalAttrLen);

        bgp::BgpUpdateNlri nlri; nlri.length = 16; nlri.prefix = v.ipv4();
        p->setNlriArraySize(1);
        p->setNlri(0, nlri);

        p->setTotalLength((uint16_t)(bgp::BGP_HEADER_OCTETS.get<B>() + 2 + p->getWithDrawnRoutesLength()
                + 2 + p->getTotalPathAttributeLength() + prefixWireBytes(16)));
        measureAndSetChunkLength(c);
    }});

    // UPDATE, RFC 4760 Multiprotocol Extensions: MP_REACH_NLRI / MP_UNREACH_NLRI advertise
    // and withdraw IPv6 reachability. The plain (IPv4) withdrawn-routes/NLRI lists are left
    // empty here -- a valid state (RFC 4271: 0 means "field not present") already exercised
    // by the "" variant above.
    fillers.push_back({"inet::bgp::BgpUpdateMessage", "mp-ipv6", [](Chunk *c) {
        auto p = check_and_cast<bgp::BgpUpdateMessage *>(c);
        FillValues v;

        auto *mpReach = new bgp::BgpUpdatePathAttributesMpReachNlri();
        mpReach->setPartialBit(false); // optional non-transitive: the flags check requires this
        mpReach->setExtendedLengthBit(true); // exercise the 2-byte length-field branch
        mpReach->setReserved(0); // required 0 by the deserializer, see the "" variant's note
        mpReach->setAfi(2); // IPv6
        mpReach->setSafi(1); // unicast
        mpReach->setNextHopLength(16);
        mpReach->setNextHop(v.l3Ipv6());
        bgp::BgpUpdateNlri6 mpNlri;
        mpNlri.length = 64;
        mpNlri.prefix = v.l3Ipv6();
        mpReach->setNlriArraySize(1);
        mpReach->setNlri(0, mpNlri);
        // value = afi(2) + safi(1) + nextHopLenField(1) + nextHop(16) + reserved-octet(1, not
        // modelled -- the serializer writes/reads it inline) + nlri(1-byte length + 8 prefix bytes)
        unsigned mpReachValueLen = 2 + 1 + 1 + 16 + 1 + (1 + 8);
        mpReach->setLength(mpReachValueLen);

        auto *mpUnreach = new bgp::BgpUpdatePathAttributesMpUnreachNlri();
        mpUnreach->setPartialBit(false);
        mpUnreach->setExtendedLengthBit(true);
        mpUnreach->setReserved(0);
        mpUnreach->setAfi(2);
        mpUnreach->setSafi(1);
        bgp::BgpUpdateNlri6 mpWr;
        mpWr.length = 48;
        mpWr.prefix = v.l3Ipv6();
        mpUnreach->setWithdrawnRoutesArraySize(1);
        mpUnreach->setWithdrawnRoutes(0, mpWr);
        unsigned mpUnreachValueLen = 2 + 1 + (1 + 6);
        mpUnreach->setLength(mpUnreachValueLen);

        p->setPathAttributesArraySize(2);
        p->setPathAttributes(0, mpReach);
        p->setPathAttributes(1, mpUnreach);
        unsigned totalAttrLen = (4 + mpReachValueLen) + (4 + mpUnreachValueLen); // extended-length header is 4 B
        p->setTotalPathAttributeLength((uint16_t)totalAttrLen);

        p->setTotalLength((uint16_t)(bgp::BGP_HEADER_OCTETS.get<B>() + 2 + 0 + 2 + totalAttrLen));
        measureAndSetChunkLength(c);
    }});

    // ===================== PIM =====================
    // Hello: one option of each type the serializer implements (AddressList/24 has no
    // branch in PimPacketSerializer's option switch and would throw -- not exercised).
    fillers.push_back({"inet::PimHello", "", [](Chunk *c) {
        auto p = check_and_cast<PimHello *>(c);
        FillValues v;
        fillPimHeader(p, v);
        auto *holdtime = new HoldtimeOption();
        holdtime->setHoldTime(v.u16());
        auto *lpd = new LanPruneDelayOption();
        lpd->setT(true); // PimPacketSerializer writes/reads the T bit as a hardcoded 0 (see report): inert
        lpd->setPropagationDelay(v.uint(15));
        lpd->setOverrideInterval(v.u16());
        auto *drp = new DrPriorityOption();
        drp->setPriority(v.u32());
        auto *genid = new GenerationIdOption();
        genid->setGenerationID(v.u32());
        p->setOptionsArraySize(4);
        p->setOptions(0, holdtime);
        p->setOptions(1, lpd);
        p->setOptions(2, drp);
        p->setOptions(3, genid);
        measureAndSetChunkLength(c);
    }});

    // JoinPrune: one non-empty join/prune group (both a joined and a pruned source), IPv4
    // and IPv6 Encoded-Address forms.
    fillers.push_back({"inet::PimJoinPrune", "", [](Chunk *c) {
        auto p = check_and_cast<PimJoinPrune *>(c);
        FillValues v;
        fillPimHeader(p, v);
        fillJoinPruneBody(p, v, false, false);
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::PimJoinPrune", "ipv6", [](Chunk *c) {
        auto p = check_and_cast<PimJoinPrune *>(c);
        FillValues v;
        fillPimHeader(p, v);
        fillJoinPruneBody(p, v, true, false);
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::PimAssert", "", [](Chunk *c) {
        auto p = check_and_cast<PimAssert *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, false));
        p->setSourceAddress(mkUnicast(v, false));
        p->setR(v.flag());
        p->setMetric(v.u32());
        p->setMetricPreference(v.uint(31));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::PimAssert", "ipv6", [](Chunk *c) {
        auto p = check_and_cast<PimAssert *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, true));
        p->setSourceAddress(mkUnicast(v, true));
        p->setR(v.flag());
        p->setMetric(v.u32());
        p->setMetricPreference(v.uint(31));
        measureAndSetChunkLength(c);
    }});

    // Graft: a normal (full-body) Graft, a body-less Graft (the serializer's "some senders"
    // shortcut: only the 4-byte common header, gated by chunkLength <= 4), and a GraftAck
    // (same wire shape as Graft, merged into it by the type discriminator).
    fillers.push_back({"inet::PimGraft", "", [](Chunk *c) {
        auto p = check_and_cast<PimGraft *>(c);
        FillValues v;
        fillPimHeader(p, v);
        // type=Graft and holdTime=0 are pinned by the .msg; the serializer also hardcodes
        // the wire holdTime of Graft/GraftAck to 0 regardless of the field's value, so
        // holdTime is intentionally left at 0 (see report).
        fillJoinPruneBody(p, v, false, true);
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::PimGraft", "bodyless", [](Chunk *c) {
        auto p = check_and_cast<PimGraft *>(c);
        FillValues v;
        fillPimHeader(p, v);
        // chunkLength == B(4) makes PimPacketSerializer skip the body entirely
        // ("a body-less Graft ... is carried by some senders"); the body fields stay at
        // their default since they are not on the wire in this form, and are exercised by
        // the "" and "graftack" variants instead.
        setChunkLength(c, B(4));
    }});
    fillers.push_back({"inet::PimGraft", "graftack", [](Chunk *c) {
        auto p = check_and_cast<PimGraft *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setType(GraftAck); // typed setter: 'type' is @editable(false)
        fillJoinPruneBody(p, v, false, true);
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::PimRegister", "", [](Chunk *c) {
        auto p = check_and_cast<PimRegister *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setB(v.flag());
        p->setN(v.flag());
        p->setReserved2(v.u32()); // not wired to the serializer at all (see report)
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::PimRegisterStop", "", [](Chunk *c) {
        auto p = check_and_cast<PimRegisterStop *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, false));
        p->setSourceAddress(mkUnicast(v, false));
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::PimRegisterStop", "ipv6", [](Chunk *c) {
        auto p = check_and_cast<PimRegisterStop *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, true));
        p->setSourceAddress(mkUnicast(v, true));
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::PimStateRefresh", "", [](Chunk *c) {
        auto p = check_and_cast<PimStateRefresh *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, false));
        p->setSourceAddress(mkUnicast(v, false));
        p->setOriginatorAddress(mkUnicast(v, false));
        // PimPacketSerializer::deserialize reads R but never restores it for StateRefresh
        // (see report): a true value here would fail the byte round-trip, so leave it false.
        p->setR(false);
        p->setMetricPreference(v.uint(31));
        p->setMetric(v.u32());
        p->setMaskLen(v.u8());
        p->setTtl(v.u8());
        p->setP(v.flag());
        p->setN(v.flag());
        p->setO(v.flag());
        p->setReserved2(v.uint(5));
        p->setInterval(v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::PimStateRefresh", "ipv6", [](Chunk *c) {
        auto p = check_and_cast<PimStateRefresh *>(c);
        FillValues v;
        fillPimHeader(p, v);
        p->setGroupAddress(mkGroup(v, true));
        p->setSourceAddress(mkUnicast(v, true));
        p->setOriginatorAddress(mkUnicast(v, true));
        p->setR(false); // see the "" variant's note
        p->setMetricPreference(v.uint(31));
        p->setMetric(v.u32());
        p->setMaskLen(v.u8());
        p->setTtl(v.u8());
        p->setP(v.flag());
        p->setN(v.flag());
        p->setO(v.flag());
        p->setReserved2(v.uint(5));
        p->setInterval(v.u8());
        measureAndSetChunkLength(c);
    }});

    // ===================== RIP =====================
    fillers.push_back({"inet::RipPacket", "", [](Chunk *c) {
        auto p = check_and_cast<RipPacket *>(c);
        FillValues v;
        p->setCommand(RIP_RESPONSE);
        p->setVersion(2);
        p->setUnused1(0); // RipPacketSerializer.cc: "'must be zero' field (RFC 2453)"
        RipEntry e0;
        e0.addressFamilyId = RIP_AF_INET;
        e0.routeTag = v.u16();
        e0.address = v.l3Ipv4();
        e0.prefixLength = 24;
        e0.nextHop = v.l3Ipv4();
        e0.metric = v.u32();
        RipEntry e1;
        e1.addressFamilyId = RIP_AF_INET;
        e1.routeTag = v.u16();
        e1.address = v.l3Ipv4();
        e1.prefixLength = 16;
        e1.nextHop = v.l3Ipv4();
        e1.metric = v.u32();
        p->setEntryArraySize(2);
        p->setEntry(0, e0);
        p->setEntry(1, e1);
        measureAndSetChunkLength(c);
    }});

    // ===================== DSDV =====================
    fillers.push_back({"inet::DsdvHello", "", [](Chunk *c) {
        auto p = check_and_cast<DsdvHello *>(c);
        FillValues v;
        p->setSrcAddress(v.ipv4());
        p->setSequencenumber(v.u32());
        p->setNextAddress(v.ipv4());
        p->setHopdistance((int)v.u32());
        measureAndSetChunkLength(c); // DsdvHello.msg declares no chunkLength default
    }});
}

} // namespace inet
