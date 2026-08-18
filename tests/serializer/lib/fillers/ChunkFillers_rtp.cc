//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for RTP/RTCP: the RTP data header (and its MPEG audio/video profile
// payload header) and the four concrete RTCP packet types (the abstract RtcpPacket
// dispatch base is on chunkSkipTypes). RtcpPacketSerializer derives from
// FieldsChunkSerializer, so measureAndSetChunkLength() can be used for the two
// types whose length the serializer computes from variable content (the RTP CSRC
// list, the padded SDES chunks) instead of hand-deriving the formula.
//

#include "../ChunkFillers.h"
#include "inet/transportlayer/rtp/RtcpPacket_m.h"
#include "inet/transportlayer/rtp/Reports_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/Sdes.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpMpegPacket_m.h"

namespace inet {

namespace {

// One reception-report block, distinct per call so two blocks in the same list
// differ. packetsLostCumulative is a 24-bit wire field kept in a plain (32-bit)
// `int` model field; v.uint(24) never sets the top byte, so the value round-trips
// exactly through writeUint24Be/readUint24Be.
rtp::ReceptionReport *makeReceptionReport(FillValues& v)
{
    auto *rr = new rtp::ReceptionReport();
    rr->setSsrc(v.u32());
    rr->setFractionLost(v.u8());
    rr->setPacketsLostCumulative(v.uint(24));
    rr->setSequenceNumber(v.u32());
    rr->setJitter(v.u32());
    rr->setLastSR(v.u32());
    rr->setDelaySinceLastSR(v.u32());
    return rr;
}

} // namespace

void addFillers_rtp(std::vector<ChunkFiller>& fillers)
{
    fillers.push_back({"inet::rtp::RtpHeader", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtpHeader *>(c);
        FillValues v;
        p->setVersion(2); // RTP version is always 2
        p->setPaddingFlag(v.flag());
        p->setExtensionFlag(v.flag());
        p->setMarker(v.flag());
        p->setPayloadType(v.uint(7));
        p->setSequenceNumber(v.u16());
        p->setTimeStamp(v.u32());
        p->setSsrc(v.u32());
        // The .msg comment claims csrc "can't be used because csrcCount is always 0",
        // but the serializer derives the on-wire cc nibble straight from the array
        // size and round-trips whatever is there -- exercise the non-empty case.
        p->setCsrcArraySize(3);
        for (size_t i = 0; i < 3; i++)
            p->setCsrc(i, v.u32());
        // Content-dependent length (12 B header + 4 B per CSRC entry); the .msg
        // default is the fixed-CSRC-free 12 B case only.
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::rtp::RtpMpegHeader", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtpMpegHeader *>(c);
        FillValues v;
        p->setHeaderLength(4); // RFC 2250: the MPEG A/V-specific header is always 4 B
        // Only the low 16 bits of this (32-bit model) field reach the wire -- see
        // RtpMpegPacketSerializer, which writes it with writeUint16Be().
        p->setPayloadLength(v.u16());
        p->setMbz(0); // RFC 2250: "Unused. Must be set to zero"
        // The remaining fields are modelled from RFC 2250 but the serializer's
        // encoding of them is commented out (only payloadLength/pictureType are
        // actually on the wire); fill them anyway so they are not silently unfilled --
        // a filler-side value here can never desync the byte round trip.
        p->setTwo(v.flag());
        p->setTemporalReference(v.uint(10));
        p->setActiveN(v.flag());
        p->setNewPictureHeader(v.flag());
        p->setSequenceHeaderPresent(v.flag());
        p->setBeginningOfSlice(v.flag());
        p->setEndOfSlice(v.flag());
        p->setPictureType(v.u8());
        p->setFbv(v.flag());
        p->setBfc(v.uint(3));
        p->setFfv(v.flag());
        p->setFfc(v.uint(3));
    }});

    fillers.push_back({"inet::rtp::RtcpByePacket", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtcpByePacket *>(c);
        FillValues v;
        p->setVersion(2); // RTP/RTCP version is always 2
        p->setPadding(v.flag());
        p->setCount(1); // exactly one SSRC follows the header -- the .msg-pinned shape
        p->setRtcpLength(v.u16());
        p->setSsrc(v.u32());
        // packetType is pinned to RTCP_PT_BYE and chunkLength to the fixed 8 B
        // (4 B header + 4 B ssrc) by the .msg defaults; the serializer neither reads
        // nor writes anything beyond that (no extra SSRCs/reason string modelled).
    }});

    fillers.push_back({"inet::rtp::RtcpReceiverReportPacket", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtcpReceiverReportPacket *>(c);
        FillValues v;
        p->setVersion(2);
        p->setPadding(v.flag());
        p->setRtcpLength(v.u16());
        p->setSsrc(v.u32());
        // addReceptionReport() bumps count and chunkLength itself (+24 B/report), so
        // neither needs to be (or should be) touched here.
        for (int i = 0; i < 2; i++)
            p->addReceptionReport(makeReceptionReport(v));
    }});

    fillers.push_back({"inet::rtp::RtcpSenderReportPacket", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtcpSenderReportPacket *>(c);
        FillValues v;
        p->setVersion(2);
        p->setPadding(v.flag());
        p->setRtcpLength(v.u16());
        p->setSsrc(v.u32());
        auto& sr = p->getSenderReportForUpdate(); // fixed 20 B sub-struct, no length change
        sr.setNTPTimeStamp(v.u64());
        sr.setRTPTimeStamp(v.u32());
        sr.setPacketCount(v.u32());
        sr.setByteCount(v.u32());
        for (int i = 0; i < 2; i++)
            p->addReceptionReport(makeReceptionReport(v));
    }});

    // SDES: two chunks (SSRC-keyed) with two items each, covering four of the SDES
    // item types. addSDESChunk() bumps count and chunkLength by the chunk's own
    // getLength() (ssrc + items), but the serializer also appends a terminating END
    // byte and pads *each chunk* independently to a 32-bit boundary
    // (RtcpPacketSerializer::serializeSdesChunk) -- addSDESChunk cannot know about
    // that, so the auto-grown chunkLength undercounts it. Measure the real length
    // instead of hand-deriving the per-chunk padding formula.
    fillers.push_back({"inet::rtp::RtcpSdesPacket", "", [](Chunk *c) {
        auto p = check_and_cast<rtp::RtcpSdesPacket *>(c);
        FillValues v;
        p->setVersion(2);
        p->setPadding(v.flag());
        p->setRtcpLength(v.u16());
        auto *chunk0 = new rtp::SdesChunk("sdes0", v.u32());
        chunk0->addSDESItem(new rtp::SdesItem(rtp::SdesItem::SDES_CNAME, v.text("cname").c_str()));
        chunk0->addSDESItem(new rtp::SdesItem(rtp::SdesItem::SDES_NAME, v.text("name").c_str()));
        p->addSDESChunk(chunk0);
        auto *chunk1 = new rtp::SdesChunk("sdes1", v.u32());
        chunk1->addSDESItem(new rtp::SdesItem(rtp::SdesItem::SDES_EMAIL, v.text("email").c_str()));
        chunk1->addSDESItem(new rtp::SdesItem(rtp::SdesItem::SDES_PHONE, v.text("phone").c_str()));
        p->addSDESChunk(chunk1);
        // the serializer accepts any declared length (it pads the tail), so the length
        // cannot be measured from it: each SDES chunk is its own item bytes plus a
        // terminating zero octet, padded up to a 32-bit boundary, after the 4-byte header
        B len = B(4);
        for (const rtp::SdesChunk *sc : { chunk0, chunk1 }) {
            int bytes = sc->getLength() + 1;
            if (bytes % 4 != 0)
                bytes += 4 - bytes % 4;
            len += B(bytes);
        }
        setChunkLength(c, len);
    }});
}

} // namespace inet
