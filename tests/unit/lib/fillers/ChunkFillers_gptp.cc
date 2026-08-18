//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for gPTP (IEEE 802.1AS): the common header (GptpBase, shared by
// every message) plus the six concrete message types. GptpBase itself is on
// chunkSkipTypes (messageType is the dispatch discriminator, left at its -1
// sentinel on a bare instance). clocktime_t fields need SIMTIME_AS_CLOCKTIME() --
// with the Clock feature enabled (as here) it is a distinct type from simtime_t
// with no implicit conversion from FillValues::time().
//

#include "../ChunkFillers.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"

namespace inet {

namespace {

void fillPortIdentity(PortIdentity& pid, FillValues& v)
{
    pid.clockIdentity = v.u64();
    pid.portNumber = v.u16();
}

// Fills every field of the common gPTP header, including the sourcePortIdentity
// sub-struct (a plain struct, no compound setter needed beyond setSourcePortIdentity).
// Left untouched, with a short reason each:
//  - messageType: @editable(false) subtype discriminator, already pinned to the
//    right GPTPTYPE_* by the concrete subclass's .msg default.
//  - minorVersionPTP / versionPTP / controlField: IEEE 802.1AS-2020 10.6.2.1/11.4.2
//    fixes these at 1 / 2 / 0 for every transmitted message -- set explicitly below
//    rather than randomized, even though that keeps them at their .msg default.
//  - messageLengthField: already the correct fixed total (from the concrete
//    subclass's GPTP_..._PACKET_SIZE default) -- a content-dependent recompute has
//    nothing to add for these fixed-size messages.
void fillGptpBase(GptpBase *p, FillValues& v)
{
    p->setMajorSdoId(v.uint(4));
    p->setMinorVersionPTP(1); // spec-fixed
    p->setVersionPTP(2);      // spec-fixed
    p->setDomainNumber(v.u8());
    p->setMinorSdoId(v.u8());
    p->setFlags(v.u16());
    p->setCorrectionField(SIMTIME_AS_CLOCKTIME(v.time()));
    p->setMessageTypeSpecific(v.u32());
    PortIdentity pid;
    fillPortIdentity(pid, v);
    p->setSourcePortIdentity(pid);
    p->setSequenceId(v.u16());
    p->setControlField(0); // spec-fixed ("The value is 0")
    p->setLogMessageInterval(v.u8());
}

} // namespace

void addFillers_gptp(std::vector<ChunkFiller>& fillers)
{
    fillers.push_back({"inet::GptpSync", "", [](Chunk *c) {
        auto p = check_and_cast<GptpSync *>(c);
        FillValues v;
        fillGptpBase(p, v);
        p->setOriginTimestampSeconds(v.uint(48));
        p->setOriginTimestampNanoseconds(v.u32());
    }});

    fillers.push_back({"inet::GptpFollowUp", "", [](Chunk *c) {
        auto p = check_and_cast<GptpFollowUp *>(c);
        FillValues v;
        fillGptpBase(p, v);
        p->setPreciseOriginTimestamp(SIMTIME_AS_CLOCKTIME(v.time()));
        auto& tlv = p->getFollowUpInformationTLVForUpdate();
        // tlvType / lengthField keep the .msg-pinned TLV type and fixed body size.
        tlv.setOrganizationId(0x0080C2);  // IEEE-assigned OUI used by 802.1AS TLVs (14.3 of 1588-2019)
        tlv.setOrganizationSubType(1);    // gPTP-defined subtype for the Follow_Up information TLV
        tlv.setCumulativeScaledRateOffset((int32_t)v.u32());
        tlv.setGmTimeBaseIndicator(v.u16());
        tlv.setLastGmPhaseChange(SIMTIME_AS_CLOCKTIME(v.time()));
        tlv.setScaledLastGmFreqChange((int32_t)v.u32());
    }});

    fillers.push_back({"inet::GptpPdelayReq", "", [](Chunk *c) {
        auto p = check_and_cast<GptpPdelayReq *>(c);
        FillValues v;
        fillGptpBase(p, v);
        // The 20 reserved octets are round-tripped verbatim (see GptpPacket.msg) --
        // give them distinct non-zero bytes so that is actually exercised, rather
        // than trivially passing on an all-zero buffer.
        for (size_t i = 0; i < p->getReservedArraySize(); i++)
            p->setReserved(i, v.u8());
    }});

    fillers.push_back({"inet::GptpPdelayResp", "", [](Chunk *c) {
        auto p = check_and_cast<GptpPdelayResp *>(c);
        FillValues v;
        fillGptpBase(p, v);
        p->setRequestReceiptTimestamp(SIMTIME_AS_CLOCKTIME(v.time()));
        PortIdentity pid;
        fillPortIdentity(pid, v);
        p->setRequestingPortIdentity(pid);
    }});

    fillers.push_back({"inet::GptpPdelayRespFollowUp", "", [](Chunk *c) {
        auto p = check_and_cast<GptpPdelayRespFollowUp *>(c);
        FillValues v;
        fillGptpBase(p, v);
        p->setResponseOriginTimestamp(SIMTIME_AS_CLOCKTIME(v.time()));
        PortIdentity pid;
        fillPortIdentity(pid, v);
        p->setRequestingPortIdentity(pid);
    }});

    fillers.push_back({"inet::GptpAnnounce", "", [](Chunk *c) {
        auto p = check_and_cast<GptpAnnounce *>(c);
        FillValues v;
        fillGptpBase(p, v);
        p->setOriginTimestampSeconds(v.uint(48));
        p->setOriginTimestampNanoseconds(v.u32());
        p->setCurrentUtcOffset((int16_t)v.u16());
        p->setReserved(v.u8());
        p->setGrandmasterPriority1(v.u8());
        p->setGrandmasterClockQuality(v.u32());
        p->setGrandmasterPriority2(v.u8());
        p->setGrandmasterIdentity(v.u64());
        p->setStepsRemoved(v.u16());
        p->setTimeSource(v.u8());
        // The optional PATH_TRACE TLV (a list of PTP clock identities appended after
        // the fixed 30 B body) is not modelled in GptpAnnounce.msg, so there is no
        // field here for it -- the fixed-body case is the only one to fill.
    }});
}

} // namespace inet
