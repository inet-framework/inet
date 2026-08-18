//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the MIPv6 Mobility Header subtypes (RFC 6275 Section 6.1), plus
// the RFC 5213 Proxy Mobile IPv6 extension that BindingUpdate / BindingAcknowledgement
// carry when their P-flag (proxyRegistrationFlag) is set.
//

#include "../ChunkFillers.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"
#include "inet/networklayer/mipv6/MobilityHeaderSerializer.h"

namespace inet {

void addFillers_mipv6(std::vector<ChunkFiller>& fillers)
{
    // The common Mobility Header (payload proto / header len / reserved / checksum) is
    // synthesized by the serializer itself and carries no model field besides the type
    // discriminator, so only mobilityHeaderType is set here for the fixed-layout types.
    // MobilityHeader declares no chunkLength in its .msg (it is a bare FieldsChunk), so
    // every subtype -- fixed or padded -- needs its wire size set explicitly.

    // --- BindingRefreshRequest: MH common (6) + reserved (2).
    fillers.push_back({"inet::BindingRefreshRequest", "", [](Chunk *c) {
        auto p = check_and_cast<BindingRefreshRequest *>(c);
        p->setMobilityHeaderType(BINDING_REFRESH_REQUEST); // pinned discriminator
        setChunkLength(c, B(8));
    }});

    // --- HomeTestInit: MH common (6) + reserved (2) + home init cookie (8).
    fillers.push_back({"inet::HomeTestInit", "", [](Chunk *c) {
        auto p = check_and_cast<HomeTestInit *>(c);
        FillValues v;
        p->setMobilityHeaderType(HOME_TEST_INIT); // pinned discriminator
        p->setHomeInitCookie(v.u64());
        setChunkLength(c, B(16));
    }});

    // --- HomeTest: MH common (6) + home nonce index (2) + home init cookie (8) + home keygen token (8).
    fillers.push_back({"inet::HomeTest", "", [](Chunk *c) {
        auto p = check_and_cast<HomeTest *>(c);
        FillValues v;
        p->setMobilityHeaderType(HOME_TEST); // pinned discriminator
        p->setHomeNonceIndex(v.u16());
        p->setHomeInitCookie(v.u64());
        p->setHomeKeyGenToken(v.u64());
        setChunkLength(c, B(24));
    }});

    // --- CareOfTestInit: MH common (6) + reserved (2) + care-of init cookie (8).
    fillers.push_back({"inet::CareOfTestInit", "", [](Chunk *c) {
        auto p = check_and_cast<CareOfTestInit *>(c);
        FillValues v;
        p->setMobilityHeaderType(CARE_OF_TEST_INIT); // pinned discriminator
        p->setCareOfInitCookie(v.u64());
        setChunkLength(c, B(16));
    }});

    // --- CareOfTest: MH common (6) + care-of nonce index (2) + care-of init cookie (8) + care-of keygen token (8).
    fillers.push_back({"inet::CareOfTest", "", [](Chunk *c) {
        auto p = check_and_cast<CareOfTest *>(c);
        FillValues v;
        p->setMobilityHeaderType(CARE_OF_TEST); // pinned discriminator
        p->setCareOfNonceIndex(v.u16());
        p->setCareOfInitCookie(v.u64());
        p->setCareOfKeyGenToken(v.u64());
        setChunkLength(c, B(24));
    }});

    // --- BindingError: MH common (6) + status (1) + reserved (1) + home address (16).
    fillers.push_back({"inet::BindingError", "", [](Chunk *c) {
        auto p = check_and_cast<BindingError *>(c);
        FillValues v;
        p->setMobilityHeaderType(BINDING_ERROR); // pinned discriminator
        p->setStatus(UNKNOWN_MH_TYPE);
        p->setHomeAddress(v.ipv6());
        setChunkLength(c, B(24));
    }});

    // --- BindingUpdate: two variants, since the serializer branches on the P-flag
    // (proxyRegistrationFlag, RFC 5213). "plain" is the classic MIPv6 layout (MH common
    // (6) + sequence (2) + flags/reserved (2) + lifetime (2) = 12, padded to the next
    // 8-octet Mobility Header boundary = 16). "proxy" additionally carries the
    // length-prefixed Mobile Node Identifier (NAI), the Home Network Prefix, the
    // Handoff Indicator / Access Technology Type and a timestamp; its length comes from
    // MobilityHeaderSerializer::getProxyBindingUpdateLength().
    fillers.push_back({"inet::BindingUpdate", "plain", [](Chunk *c) {
        auto p = check_and_cast<BindingUpdate *>(c);
        FillValues v;
        p->setMobilityHeaderType(BINDING_UPDATE); // pinned discriminator
        p->setSequence(v.u16());
        p->setLifetime(4u * v.uint(14)); // wire unit is 4 s (RFC 6275 6.1.7); keep it an exact multiple
        p->setAckFlag(v.flag());
        p->setHomeRegistrationFlag(v.flag());
        p->setLinkLocalAddressCompatibilityFlag(v.flag());
        p->setKeyManagementFlag(v.flag());
        p->setHomeAddressMN(v.ipv6()); // modelled but not written by the serializer (see class comment)
        p->setBindingAuthorizationData((int)v.u32()); // modelled but not written by the serializer
        p->setProxyRegistrationFlag(false); // pinned: plain MIPv6, no P-flag -- the "proxy" variant covers the P=1 fields
        setChunkLength(c, B(16));
    }});
    fillers.push_back({"inet::BindingUpdate", "proxy", [](Chunk *c) {
        auto p = check_and_cast<BindingUpdate *>(c);
        FillValues v;
        p->setMobilityHeaderType(BINDING_UPDATE); // pinned discriminator
        p->setSequence(v.u16());
        p->setLifetime(4u * v.uint(14));
        p->setAckFlag(v.flag());
        p->setHomeRegistrationFlag(v.flag());
        p->setLinkLocalAddressCompatibilityFlag(v.flag());
        p->setKeyManagementFlag(v.flag());
        p->setHomeAddressMN(v.ipv6());
        p->setBindingAuthorizationData((int)v.u32());
        p->setProxyRegistrationFlag(true); // pinned: Proxy Binding Update (RFC 5213 P-flag)
        std::string nai = v.text("mn");
        p->setMobileNodeIdentifier(nai.c_str());
        p->setHomeNetworkPrefix(v.ipv6());
        p->setHomeNetworkPrefixLength(v.u8());
        p->setHandoffIndicator(v.u8());
        p->setAccessTechnologyType(v.u8());
        p->setTimestampValue(v.u64());
        setChunkLength(c, MobilityHeaderSerializer::getProxyBindingUpdateLength(nai.size()));
    }});

    // --- BindingAcknowledgement: same P-flag split as BindingUpdate. The proxy layout
    // omits the Handoff Indicator / Access Technology Type (RFC 5213 8.1.1: a PBA
    // doesn't ack a handoff), so getProxyBindingAcknowledgementLength() is shorter than
    // the matching PBU for the same NAI.
    fillers.push_back({"inet::BindingAcknowledgement", "plain", [](Chunk *c) {
        auto p = check_and_cast<BindingAcknowledgement *>(c);
        FillValues v;
        p->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT); // pinned discriminator
        p->setStatus(BU_ACCEPT_BUT_DISCOVER_PREFIX);
        p->setSequenceNumber(v.u16());
        p->setLifetime(4u * v.uint(14));
        p->setKeyManagementFlag(v.flag());
        p->setBindingAuthorizationData((int)v.u32()); // modelled but not written by the serializer
        p->setProxyRegistrationFlag(false); // pinned: plain MIPv6, no P-flag -- the "proxy" variant covers the P=1 fields
        setChunkLength(c, B(16));
    }});
    fillers.push_back({"inet::BindingAcknowledgement", "proxy", [](Chunk *c) {
        auto p = check_and_cast<BindingAcknowledgement *>(c);
        FillValues v;
        p->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT); // pinned discriminator
        p->setStatus(TIMESTAMP_MISMATCH); // an RFC 5213 proxy-specific status (8.1.1)
        p->setSequenceNumber(v.u16());
        p->setLifetime(4u * v.uint(14));
        p->setKeyManagementFlag(v.flag());
        p->setBindingAuthorizationData((int)v.u32());
        p->setProxyRegistrationFlag(true); // pinned: Proxy Binding Acknowledgement (RFC 5213 P-flag)
        std::string nai = v.text("mn");
        p->setMobileNodeIdentifier(nai.c_str());
        p->setHomeNetworkPrefix(v.ipv6());
        p->setHomeNetworkPrefixLength(v.u8());
        p->setTimestampValue(v.u64());
        setChunkLength(c, MobilityHeaderSerializer::getProxyBindingAcknowledgementLength(nai.size()));
    }});
}

} // namespace inet
