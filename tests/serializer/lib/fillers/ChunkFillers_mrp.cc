//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for MRP (IEC 62439-2 Media Redundancy Protocol): the version field,
// the TLVs carried in an MRP_PDU (Test, TopologyChange, LinkChange, the In* ring-
// interconnection variants, Common, Option, End) and the Option's SubTLVs.
//
// Every MrpXxx type here is its own standalone, fixed-size FieldsChunk -- the
// serializer (MrpPduSerializer.cc) dispatches purely on the headerType/subType
// byte, and each TLV's .msg chunkLength already equals its wire size (2-byte TLV
// header + fixed value, 4-byte padded where the serializer pads), so none of them
// is content-dependent: no filler here calls measureAndSetChunkLength/
// setChunkLength.
//
// MrpLinkChange/MrpInLinkChange model LinkUp and LinkDown (and MrpSubTlvTest models
// TEST_MGR_NACK/TEST_PROPAGATE) as one class carrying a mutable discriminator, not
// as two subclasses; the serializer's switch groups both values into one identical
// branch (same fields, same order), so a single pinned value already exercises the
// branch -- pin one and note the alternative in a comment, no second variant.

#include "../ChunkFillers.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"

namespace inet {

void addFillers_mrp(std::vector<ChunkFiller>& fillers)
{
    fillers.push_back({"inet::MrpVersion", "", [](Chunk *c) {
        auto p = check_and_cast<MrpVersion *>(c);
        FillValues v;
        p->setVersion(v.u16());
    }});

    fillers.push_back({"inet::MrpCommon", "", [](Chunk *c) {
        auto p = check_and_cast<MrpCommon *>(c);
        FillValues v;
        p->setHeaderType(COMMON); // TLV type: pinned by subclass
        p->setValueLength(18); // sequenceID(2) + uuid0(8) + uuid1(8): must match the serialized value size
        p->setSequenceID(v.u16());
        p->setUuid0(v.u64());
        p->setUuid1(v.u64());
    }});

    fillers.push_back({"inet::MrpTest", "", [](Chunk *c) {
        auto p = check_and_cast<MrpTest *>(c);
        FillValues v;
        p->setHeaderType(TEST); // TLV type: pinned by subclass
        p->setValueLength(18); // prio(2) + sa(6) + portRole(2) + ringState(2) + transition(2) + timeStamp(4)
        p->setPrio(v.u16());
        p->setSa(v.mac());
        p->setPortRole(v.u16());
        p->setRingState(v.u16());
        p->setTransition(v.u16());
        p->setTimeStamp(v.u32());
    }});

    fillers.push_back({"inet::MrpTopologyChange", "", [](Chunk *c) {
        auto p = check_and_cast<MrpTopologyChange *>(c);
        FillValues v;
        p->setHeaderType(TOPOLOGYCHANGE); // TLV type: pinned by subclass
        p->setValueLength(10); // prio(2) + sa(6) + interval(2)
        p->setPrio(v.u16());
        p->setSa(v.mac());
        p->setInterval(v.u16());
    }});

    fillers.push_back({"inet::MrpLinkChange", "", [](Chunk *c) {
        auto p = check_and_cast<MrpLinkChange *>(c);
        FillValues v;
        p->setHeaderType(LINKDOWN); // MRP_LinkUp/MRP_LinkDown share this class and this exact wire layout; pin one (alternative: LINKUP)
        p->setValueLength(14); // sa(6) + portRole(2) + interval(2) + blocked(2) + reserved(2)
        p->setSa(v.mac());
        p->setPortRole(v.u16());
        p->setInterval(v.u16());
        p->setBlocked(v.u16());
        p->setReserved(v.u16());
    }});

    fillers.push_back({"inet::MrpInTest", "", [](Chunk *c) {
        auto p = check_and_cast<MrpInTest *>(c);
        FillValues v;
        p->setHeaderType(INTEST); // TLV type: pinned by subclass
        p->setValueLength(18); // inID(2) + sa(6) + portRole(2) + inState(2) + transition(2) + timeStamp(4)
        p->setInID(v.u16());
        p->setSa(v.mac());
        p->setPortRole(v.u16());
        p->setInState(v.u16());
        p->setTransition(v.u16());
        p->setTimeStamp(v.u32());
    }});

    fillers.push_back({"inet::MrpInTopologyChange", "", [](Chunk *c) {
        auto p = check_and_cast<MrpInTopologyChange *>(c);
        FillValues v;
        p->setHeaderType(INTOPOLOGYCHANGE); // TLV type: pinned by subclass
        p->setValueLength(10); // sa(6) + inID(2) + interval(2)
        p->setSa(v.mac());
        p->setInID(v.u16());
        p->setInterval(v.u16());
    }});

    fillers.push_back({"inet::MrpInLinkChange", "", [](Chunk *c) {
        auto p = check_and_cast<MrpInLinkChange *>(c);
        FillValues v;
        p->setHeaderType(INLINKDOWN); // MRP_InLinkUp/MRP_InLinkDown share this class and this exact wire layout; pin one (alternative: INLINKUP)
        p->setValueLength(14); // sa(6) + portRole(2) + inID(2) + interval(2) + linkInfo(2)
        p->setSa(v.mac());
        p->setPortRole(v.u16());
        p->setInID(v.u16());
        p->setInterval(v.u16());
        p->setLinkInfo(v.u16());
    }});

    fillers.push_back({"inet::MrpInLinkStatusPoll", "", [](Chunk *c) {
        auto p = check_and_cast<MrpInLinkStatusPoll *>(c);
        FillValues v;
        p->setHeaderType(INLINKSTATUSPOLL); // TLV type: pinned by subclass
        p->setValueLength(10); // sa(6) + portRole(2) + inID(2)
        p->setSa(v.mac());
        p->setPortRole(v.u16());
        p->setInID(v.u16());
    }});

    fillers.push_back({"inet::MrpOption", "", [](Chunk *c) {
        auto p = check_and_cast<MrpOption *>(c);
        FillValues v;
        p->setHeaderType(OPTION); // TLV type: pinned by subclass
        p->setValueLength(4); // ouiType(3) + ed1Type(1)
        p->setOuiType(OUI); // the other of the only two valid values (.msg default is IEC)
        // ed1Type would select the length of a following ManufacturerData/SubTlvHeader
        // payload per Table 25, but that payload is not modeled here (see MrpOption.msg
        // and MrpTlvSerializer's OPTION case, which never emits/reads it) -- so any byte
        // value round-trips; use a filled, distinct one.
        p->setEd1Type(v.u8());
    }});

    // MrpEnd carries no value (headerType only); valueLength is fixed at 0 by the
    // format (MRP_End has no payload) and is intentionally left at its .msg default
    // -- see the .test's knownUnfilled (inet::MrpEnd::valueLength).
    fillers.push_back({"inet::MrpEnd", "", [](Chunk *c) {
        auto p = check_and_cast<MrpEnd *>(c);
        p->setHeaderType(END); // TLV type: pinned by subclass
    }});

    fillers.push_back({"inet::MrpSubTlvHeader", "", [](Chunk *c) {
        auto p = check_and_cast<MrpSubTlvHeader *>(c);
        p->setSubType(AUTOMGR); // SubTLV type: pinned by subclass (RESERVED case is identical on the wire, see MrpManufacturerFkt)
        // subHeaderLength is intentionally left at its .msg default (0) -- see the
        // .test's knownUnfilled (inet::MrpSubTlvHeader::subHeaderLength).
    }});

    fillers.push_back({"inet::MrpManufacturerFkt", "", [](Chunk *c) {
        auto p = check_and_cast<MrpManufacturerFkt *>(c);
        p->setSubType(RESERVED); // SubTLV type: pinned by subclass
        // subHeaderLength is intentionally left at its .msg default (0) -- see the
        // .test's knownUnfilled (inet::MrpManufacturerFkt::subHeaderLength).
    }});

    fillers.push_back({"inet::MrpSubTlvTest", "", [](Chunk *c) {
        auto p = check_and_cast<MrpSubTlvTest *>(c);
        FillValues v;
        p->setSubType(TEST_PROPAGATE); // the discriminator is wire data: the other value has its own variant
        p->setSubHeaderLength(16); // prio(2) + sa(6) + otherMRMPrio(2) + otherMRMSa(6)
        p->setPrio(v.u16());
        p->setSa(v.mac());
        p->setOtherMRMPrio(v.u16());
        p->setOtherMRMSa(v.mac());
    }});
    fillers.push_back({"inet::MrpSubTlvTest", "mgrnack", [](Chunk *c) {
        auto p = check_and_cast<MrpSubTlvTest *>(c);
        FillValues v;
        p->setSubType(TEST_MGR_NACK); // the other value the shared wire layout carries
        p->setSubHeaderLength(16); // prio(2) + sa(6) + otherMRMPrio(2) + otherMRMSa(6)
        p->setPrio(v.u16());
        p->setSa(v.mac());
        p->setOtherMRMPrio(v.u16());
        p->setOtherMRMSa(v.mac());
    }});
}

} // namespace inet
