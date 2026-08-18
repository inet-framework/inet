//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Chunk fillers for the IEEE 802.11 MAC frame family: the MAC header hierarchy
// (control frames, the data header, the action-frame / block-ack subtypes), the
// A-MSDU/A-MPDU aggregation subframe headers, the FCS trailer, and the management
// frame bodies. Management frames are TWO chunks in this model -- Ieee80211MgmtHeader
// (the MAC header, an abstract-by-convention base filled elsewhere) and the body
// class (e.g. Ieee80211BeaconFrame) filled here.
//

#include "../ChunkFillers.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {

using namespace ieee80211;

namespace {

// Fields every Ieee80211MacHeader subtype carries. moreFragments/retry/powerMgmt/
// moreData/protectedFrame are part of the frame-control byte the serializer writes
// unconditionally for every subtype (powerMgmt/moreData/protectedFrame are marked
// "unused" in the .msg -- no MAC logic reads them -- but they are still real wire
// bits); each has a `false` default, and with a single filler per type the only way
// to differ from that default is the literal `true` (a v.flag() call could coincide
// with the default depending on call position, which is not something to gamble the
// coverage gate on). durationField is a real wire field despite the "-1 = no
// duration" .msg comment: leaving it at -1 would cast to a bogus 16-bit value.
// AID and MACArrive are technical/internal fields Ieee80211MacHeaderSerializer never
// reads or writes (no getAID()/getMACArrive() call anywhere in it); fill them too so
// they do not show up as coverage gaps instead of special-casing every subtype into
// knownUnfilled. toDS/fromDS/order/type are left to the caller: they select real
// serializer branches (address4 presence, the QoS control field, the mgmt "order"
// HT-Control field, the frame-type dispatch itself).
void fillMacHeaderCommon(Ieee80211MacHeader *h, FillValues& v)
{
    h->setMoreFragments(true);
    h->setRetry(true);
    h->setPowerMgmt(true);
    h->setMoreData(true);
    h->setProtectedFrame(true);
    h->setDurationField(v.time());
    h->setReceiverAddress(v.mac());
    h->setAID((short)v.uint(14));
    h->setMACArrive(v.time());
}

// Shared setup for the four action-frame subtypes (ActionFrameOther, AddbaRequest,
// AddbaResponse, Delba): the mac-header-common fields plus the TwoAddressHeader /
// DataOrMgmtHeader fields all four inherit, and the `type` discriminator ST_ACTION
// pinned by the Ieee80211ActionFrame base's .msg default (already the class's own
// default value, so this is a no-op for the coverage gate, but stating it keeps the
// wire discriminator visible in the filler).
void fillActionFrameHeader(Ieee80211ActionFrame *h, FillValues& v)
{
    fillMacHeaderCommon(h, v);
    h->setToDS(true);
    h->setFromDS(true);
    h->setTransmitterAddress(v.mac());
    h->setAddress3(v.mac());
    h->setFragmentNumber((short)v.uint(4));
    h->setSequenceNumber(SequenceNumberCyclic(v.uint(12)));
    h->setType(ST_ACTION);
}

// Supported-rates IE shared by the mgmt bodies below. Rates must be exact multiples
// of 0.5 Mbit/s: the serializer round-trips them through ceil(rate/0.5) on the wire
// (a 1-byte, 0.5 Mbit/s granularity encoding), so any other value would silently
// change on the first serialize. One rate carries the BSSBasicRateSet bit (the wire's
// top bit) so that branch of the per-rate encoding is exercised too.
void fillSupportedRates(Ieee80211SupportedRatesElement& sr)
{
    sr.numRates = 3;
    sr.rate[0] = 6.0;
    sr.basicRate[0] = true; // a BSS basic rate -> the 0x80 bit on the wire
    sr.rate[1] = 12.0;
    sr.basicRate[1] = false;
    sr.rate[2] = 54.0;
    sr.basicRate[2] = false;
}

} // namespace

void addFillers_ieee80211(std::vector<ChunkFiller>& fillers)
{
    // --- control frames: fixed-size headers, chunkLength keeps its .msg default. ---
    fillers.push_back({"inet::ieee80211::Ieee80211AckFrame", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211AckFrame *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setType(ST_ACK); // wire discriminator; matches the .msg default for this subclass
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211RtsFrame", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211RtsFrame *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setType(ST_RTS); // wire discriminator; matches the .msg default for this subclass
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211CtsFrame", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211CtsFrame *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setType(ST_CTS); // wire discriminator; matches the .msg default for this subclass
    }});

    // --- data header: the serializer branches on two independent things -- the QoS
    // Control field (subtype bit 0x08, i.e. type == ST_DATA_WITH_QOS) and Address4
    // (present iff toDS && fromDS) -- so all four combinations get their own case. The
    // .msg's own FIXME notes the length is not the fixed DATAFRAME_HEADER_MINLENGTH
    // default in any of them; measure it every time.
    fillers.push_back({"inet::ieee80211::Ieee80211DataHeader", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211DataHeader *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(false);
        h->setFromDS(true); // one-DS direction -> Address4 absent (see the "addr4" variants)
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setAddress3(v.mac());
        h->setFragmentNumber((short)v.uint(4));
        h->setSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        setChunkLength(c, DATAFRAME_HEADER_MINLENGTH); // 24 B: no Address4, no QoS Control
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211DataHeader", "addr4", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211DataHeader *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true); // toDS && fromDS -> Address4 present
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setAddress3(v.mac());
        h->setFragmentNumber((short)v.uint(4));
        h->setSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        h->setAddress4(v.mac());
        setChunkLength(c, DATAFRAME_HEADER_MINLENGTH + B(6)); // + Address4
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211DataHeader", "qos", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211DataHeader *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setType(ST_DATA_WITH_QOS); // selects the QoS Control field branch
        h->setToDS(false);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setAddress3(v.mac());
        h->setFragmentNumber((short)v.uint(4));
        h->setSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        h->setAckPolicy((AckPolicy)v.uint(2));
        h->setTid(v.uint(4));
        h->setEosp(false); // differs from the .msg default (true); explicit, not v.flag(),
                            // so the coverage gate sees the change regardless of call order
        h->setAMsduPresent(true);
        setChunkLength(c, DATAFRAME_HEADER_MINLENGTH + B(2)); // + QoS Control
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211DataHeader", "qos_addr4", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211DataHeader *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setType(ST_DATA_WITH_QOS);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setAddress3(v.mac());
        h->setFragmentNumber((short)v.uint(4));
        h->setSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        h->setAckPolicy((AckPolicy)v.uint(2));
        h->setTid(v.uint(4));
        h->setEosp(true);
        h->setAMsduPresent(true);
        h->setAddress4(v.mac());
        setChunkLength(c, DATAFRAME_HEADER_MINLENGTH + B(6) + B(2)); // + Address4 + QoS Control
    }});

    // --- A-MSDU / A-MPDU aggregation subframe headers: `length` describes the
    // following subframe, not this header, so it is plain wire data. MPDU length is a
    // 12-bit field.
    fillers.push_back({"inet::ieee80211::Ieee80211MsduSubframeHeader", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211MsduSubframeHeader *>(c);
        FillValues v;
        h->setSa(v.mac());
        h->setDa(v.mac());
        h->setLength(v.u16());
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211MpduSubframeHeader", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211MpduSubframeHeader *>(c);
        FillValues v;
        h->setLength((int)v.uint(12));
    }});

    // --- FCS trailer: commonSetup already sets fcsMode to FCS_COMPUTED.
    fillers.push_back({"inet::ieee80211::Ieee80211MacTrailer", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211MacTrailer *>(c);
        FillValues v;
        h->setFcs(v.u32());
    }});

    // --- action frames: an unmodelled category/action (ActionFrameOther, whose whole
    // action body is preserved verbatim) plus the three Block-Ack actions the model
    // does parse (ADDBA request/response, DELBA). Each gets an "order" variant: the
    // mgmt/action header branch emits an extra 4-byte HT Control field when the frame
    // control Order bit is set, growing the frame past the .msg's fixed chunkLength --
    // the shared branch is exercised once per type; the base variant keeps order off
    // so the fixed length stays valid.
    fillers.push_back({"inet::ieee80211::Ieee80211ActionFrameOther", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ActionFrameOther *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(false);
        // category/blockAckAction are not written to the wire for this subtype -- the
        // whole action body is dumped verbatim from actionBody instead -- but fill them
        // anyway so they are not reported as coverage gaps.
        p->setCategory((short)v.u8());
        p->setBlockAckAction(v.u8());
        p->setActionBodyArraySize(3);
        // actionBody[0] must not be 3 or the header deserializer would decode this as a
        // Block Ack action frame instead of round-tripping the raw body; v.u8() never
        // returns a value == 3 (its values are all == 1 mod 4).
        p->setActionBody(0, v.u8());
        p->setActionBody(1, v.u8());
        p->setActionBody(2, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211ActionFrameOther", "order", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ActionFrameOther *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(true);
        p->setCategory((short)v.u8());
        p->setBlockAckAction(v.u8());
        p->setActionBodyArraySize(3);
        p->setActionBody(0, v.u8());
        p->setActionBody(1, v.u8());
        p->setActionBody(2, v.u8());
        measureAndSetChunkLength(c);
    }});

    fillers.push_back({"inet::ieee80211::Ieee80211AddbaRequest", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AddbaRequest *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(false); // keeps the fixed .msg chunkLength valid; see the "order" variant
        p->setCategory(3); // wire discriminator; matches the .msg default (Block Ack category)
        p->setBlockAckAction(0); // wire discriminator; matches the .msg default (ADDBA request)
        p->setDialogToken(v.u8());
        p->setAMsduSupported(true);
        p->setBlockAckPolicy(true);
        p->setTid(v.uint(4));
        p->setBufferSize(v.uint(10));
        p->setBlockAckTimeoutValue(SimTime(v.uint(8) * 1024, SIMTIME_US)); // must be a multiple of 1024 us
        // _fragmentNumber: the Fragment Number subfield of the Block Ack Starting
        // Sequence Control is always 0 in a Block Ack context (no fragmentation);
        // kept at the .msg default.
        p->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211AddbaRequest", "order", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AddbaRequest *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(true);
        p->setCategory(3);
        p->setBlockAckAction(0);
        p->setDialogToken(v.u8());
        p->setAMsduSupported(true);
        p->setBlockAckPolicy(true);
        p->setTid(v.uint(4));
        p->setBufferSize(v.uint(10));
        p->setBlockAckTimeoutValue(SimTime(v.uint(8) * 1024, SIMTIME_US));
        p->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        setChunkLength(c, LENGTH_ADDBAREQ); // +4 B for the HT Control field order=1 adds
    }});

    fillers.push_back({"inet::ieee80211::Ieee80211AddbaResponse", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AddbaResponse *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(false);
        p->setCategory(3);
        p->setBlockAckAction(1); // wire discriminator; matches the .msg default (ADDBA response)
        p->setDialogToken(v.u8());
        p->setStatusCode(v.u16());
        p->setAMsduSupported(true);
        p->setBlockAckPolicy(true);
        p->setTid(v.uint(4));
        p->setBufferSize(v.uint(10));
        p->setBlockAckTimeoutValue(SimTime(v.uint(8) * 1024, SIMTIME_US));
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211AddbaResponse", "order", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AddbaResponse *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(true);
        p->setCategory(3);
        p->setBlockAckAction(1);
        p->setDialogToken(v.u8());
        p->setStatusCode(v.u16());
        p->setAMsduSupported(true);
        p->setBlockAckPolicy(true);
        p->setTid(v.uint(4));
        p->setBufferSize(v.uint(10));
        p->setBlockAckTimeoutValue(SimTime(v.uint(8) * 1024, SIMTIME_US));
        setChunkLength(c, LENGTH_ADDBARESP);
    }});

    fillers.push_back({"inet::ieee80211::Ieee80211Delba", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211Delba *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(false);
        p->setCategory(3);
        p->setBlockAckAction(2); // wire discriminator; matches the .msg default (DELBA)
        p->setReserved(v.uint(11));
        p->setInitiator(true);
        p->setTid(v.uint(4));
        p->setReasonCode(v.u16());
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211Delba", "order", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211Delba *>(c);
        FillValues v;
        fillActionFrameHeader(p, v);
        p->setOrder(true);
        p->setCategory(3);
        p->setBlockAckAction(2);
        p->setReserved(v.uint(11));
        p->setInitiator(true);
        p->setTid(v.uint(4));
        p->setReasonCode(v.u16());
        setChunkLength(c, LENGTH_DELBA);
    }});

    // --- Block Ack request / response, Basic and Compressed variants: multiTid and
    // compressedBitmap are the serializer's variant discriminator (see
    // Ieee80211MacHeaderSerializer's ST_BLOCKACK_REQ/ST_BLOCKACK cases), pinned per
    // subclass by the .msg -- kept at that pinned pair. fragmentNumber is the Fragment
    // Number subfield of the Block Ack (Req) Starting Sequence Control, which 802.11
    // always sets to 0 in a Block Ack context (no fragmentation) -- kept at the .msg
    // default in all four subtypes.
    fillers.push_back({"inet::ieee80211::Ieee80211BasicBlockAckReq", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211BasicBlockAckReq *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setType(ST_BLOCKACK_REQ); // wire discriminator; matches the BlockAckReq base's .msg default
        h->setMultiTid(false); // pinned by the class: selects the (non-multi-TID) Basic/Compressed branch
        h->setCompressedBitmap(false); // pinned by the class: selects the Basic (non-compressed) branch
        h->setBarAckPolicy(true);
        h->setReserved(v.uint(9));
        h->setTidInfo((int)v.uint(4));
        h->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211CompressedBlockAckReq", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211CompressedBlockAckReq *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setType(ST_BLOCKACK_REQ);
        h->setMultiTid(false); // pinned by the class
        h->setCompressedBitmap(true); // pinned by the class: selects the Compressed branch
        h->setBarAckPolicy(true);
        h->setReserved(v.uint(9));
        h->setTidInfo((int)v.uint(4));
        h->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211BasicBlockAck", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211BasicBlockAck *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setType(ST_BLOCKACK); // wire discriminator; matches the BlockAck base's .msg default
        h->setMultiTid(false); // pinned by the class
        h->setCompressedBitmap(false); // pinned by the class: selects the Basic bitmap branch
        h->setBlockAckPolicy(true);
        h->setReserved(v.uint(9));
        h->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        // The Block Ack bitmap: 64 elements of 2 bytes (16 bits) each; never leave any at
        // the default empty BitVector -- getBytes()[0]/[1] would read out of bounds.
        for (size_t i = 0; i < 64; i++)
            h->setBlockAckBitmap(i, BitVector(std::vector<uint8_t>{v.u8(), v.u8()}));
        h->setTidInfo((int)v.uint(4));
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211CompressedBlockAck", "", [](Chunk *c) {
        auto h = check_and_cast<Ieee80211CompressedBlockAck *>(c);
        FillValues v;
        fillMacHeaderCommon(h, v);
        h->setToDS(true);
        h->setFromDS(true);
        h->setOrder(true);
        h->setTransmitterAddress(v.mac());
        h->setType(ST_BLOCKACK);
        h->setMultiTid(false); // pinned by the class
        h->setCompressedBitmap(true); // pinned by the class: selects the Compressed bitmap branch
        h->setBlockAckPolicy(true);
        h->setReserved(v.uint(9));
        h->setStartingSequenceNumber(SequenceNumberCyclic(v.uint(12)));
        h->setBlockAckBitmap(BitVector(std::vector<uint8_t>{v.u8(), v.u8(), v.u8(), v.u8(), v.u8(), v.u8(), v.u8(), v.u8()}));
        h->setTidInfo((int)v.uint(4));
        // the .msg leaves chunkLength unset ("TODO"); the wire layout is fixed at
        // 16 (header) + 2 (BA control) + 2 (starting-sequence control) + 8 (compressed
        // bitmap) = 28 bytes.
        setChunkLength(c, B(28));
    }});

    // --- management frame bodies. Each is content-length-dependent (SSID / rates /
    // trailingBytes), so all measure their chunkLength. trailingBytes preserves
    // information elements this model does not represent (see the .msg); fill it with
    // a couple of bytes so the "re-emit the trailing bytes verbatim" loop is exercised
    // -- an empty array would leave that loop dead code.
    fillers.push_back({"inet::ieee80211::Ieee80211AuthenticationFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AuthenticationFrame *>(c);
        FillValues v;
        p->setSequenceNumber(v.u16());
        p->setStatusCode(SC_AUTH_ALG0_UNSUP);
        // isLast is explicitly not part of the standard (see the .msg comment) and is
        // never read or written by the serializer; left at its default.
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211DeauthenticationFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211DeauthenticationFrame *>(c);
        FillValues v;
        p->setReasonCode(RC_DEAUTH_MS_LEAVING);
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211DisassociationFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211DisassociationFrame *>(c);
        FillValues v;
        p->setReasonCode(RC_DISASS_INACTIVITY);
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211ProbeRequestFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ProbeRequestFrame *>(c);
        FillValues v;
        p->setSSID(v.text("ssid").c_str());
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211AssociationRequestFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AssociationRequestFrame *>(c);
        FillValues v;
        p->setSSID(v.text("ssid").c_str());
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setCapabilityInformation(v.u16());
        p->setListenInterval(v.u16());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211ReassociationRequestFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ReassociationRequestFrame *>(c);
        FillValues v;
        p->setSSID(v.text("ssid").c_str());
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setCapabilityInformation(v.u16());
        p->setListenInterval(v.u16());
        p->setCurrentAP(v.mac());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211AssociationResponseFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211AssociationResponseFrame *>(c);
        FillValues v;
        p->setStatusCode(SC_ASS_DENIED_UNKNOWN);
        p->setAid((short)v.uint(14));
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setCapabilityInformation(v.u16());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211ReassociationResponseFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ReassociationResponseFrame *>(c);
        FillValues v;
        p->setStatusCode(SC_REASS_DENIED);
        p->setAid((short)v.uint(14));
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setCapabilityInformation(v.u16());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211BeaconFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211BeaconFrame *>(c);
        FillValues v;
        p->setSSID(v.text("ssid").c_str());
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setBeaconInterval(SimTime(102400, SIMTIME_US)); // must be a multiple of 1024 us
        // channelNumber and handoverParameters are not part of the standard frame and
        // are never read or written by Ieee80211MgmtFrameSerializer; left at default.
        p->setTimestamp(v.u64()); // never 0 (FillValues), so the serializer's simTime()
                                  // fallback for simulation-built (timestamp==0) beacons
                                  // is not taken -- this value round-trips verbatim
        p->setCapabilityInformation(v.u16());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
    fillers.push_back({"inet::ieee80211::Ieee80211ProbeResponseFrame", "", [](Chunk *c) {
        auto p = check_and_cast<Ieee80211ProbeResponseFrame *>(c);
        FillValues v;
        p->setSSID(v.text("ssid").c_str());
        fillSupportedRates(p->getSupportedRatesForUpdate());
        p->setBeaconInterval(SimTime(51200, SIMTIME_US)); // must be a multiple of 1024 us
        p->setTimestamp(v.u64());
        p->setCapabilityInformation(v.u16());
        p->setTrailingBytesArraySize(2);
        p->setTrailingBytes(0, v.u8());
        p->setTrailingBytes(1, v.u8());
        measureAndSetChunkLength(c);
    }});
}

} // namespace inet
