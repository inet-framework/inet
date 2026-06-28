//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//
// Wire format (RFC 6126, section 4): a Babel packet is a 4-byte header followed
// by a sequence of TLVs. Each TLV is modelled as its own FieldsChunk and
// carries wire-level primitive fields; the 64-bit router-id is split into hi/lo
// words and a prefix is carried as a (masked) L3Address plus its length. The
// addresses are encoded on the wire per the AE (address-encoding) field by the
// BabelSerializer, which also performs link-local compression (AE 3).
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet::babel;

//
// TLV type codes (RFC 6126, section 4.5).
//
enum BabelTlvType
{
    BABEL_PAD1 = 0;
    BABEL_PADN = 1;
    BABEL_ACKREQ = 2;
    BABEL_ACK = 3;
    BABEL_HELLO = 4;
    BABEL_IHU = 5;
    BABEL_ROUTERID = 6;
    BABEL_NEXTHOP = 7;
    BABEL_UPDATE = 8;
    BABEL_ROUTEREQ = 9;
    BABEL_SEQNOREQ = 10;
}

//
// Babel message header (RFC 6126, section 4.2): magic 42, version 2, and the
// length in bytes of the TLV body that follows.
//
class BabelHeader extends FieldsChunk
{
    chunkLength = B(4);
    uint8_t magic = 42;
    uint8_t version = 2;
    uint16_t bodyLength = 0;
}

//
// Common base for all Babel TLVs (RFC 6126, section 4.3).
//
class BabelTlv extends FieldsChunk
{
    uint8_t tlvType; // ~BabelTlvType
}

//
// Pad1 (type 0): a single zero octet; the only TLV without a length octet.
//
class BabelPad1Tlv extends BabelTlv
{
    chunkLength = B(1);
    tlvType = BABEL_PAD1;
}

//
// PadN (type 1): n zero octets of padding (chunkLength = 2 + n).
//
class BabelPadNTlv extends BabelTlv
{
    tlvType = BABEL_PADN;
    uint8_t n = 0;
}

//
// Acknowledgment Request (type 2).
//
class BabelAckReqTlv extends BabelTlv
{
    chunkLength = B(8);
    tlvType = BABEL_ACKREQ;
    uint16_t reserved = 0;
    uint16_t nonce = 0;
    uint16_t interval = 0; // centiseconds
}

//
// Acknowledgment (type 3).
//
class BabelAckTlv extends BabelTlv
{
    chunkLength = B(4);
    tlvType = BABEL_ACK;
    uint16_t nonce = 0;
}

//
// Hello (type 4).
//
class BabelHelloTlv extends BabelTlv
{
    chunkLength = B(8);
    tlvType = BABEL_HELLO;
    uint16_t reserved = 0;
    uint16_t seqno = 0;
    uint16_t interval = 0; // centiseconds
}

//
// I Heard You (type 5): reports the rxcost the sender computed for a neighbour.
//
class BabelIhuTlv extends BabelTlv
{
    tlvType = BABEL_IHU;
    uint8_t ae = 0; // ~BabelAddressEncoding
    uint8_t reserved = 0;
    uint16_t rxcost = 0;
    uint16_t interval = 0; // centiseconds
    L3Address address; // the neighbour's address (encoded per ae)
}

//
// Router-Id (type 6): establishes the router-id for subsequent Update TLVs.
//
class BabelRouterIdTlv extends BabelTlv
{
    chunkLength = B(12);
    tlvType = BABEL_ROUTERID;
    uint16_t reserved = 0;
    uint32_t routerIdHi = 0;
    uint32_t routerIdLo = 0;
}

//
// Next Hop (type 7): establishes the next hop for subsequent Update TLVs.
//
class BabelNextHopTlv extends BabelTlv
{
    tlvType = BABEL_NEXTHOP;
    uint8_t ae = 0;
    uint8_t reserved = 0;
    L3Address nextHop;
}

//
// Update (type 8): announces (or, with infinite metric, retracts) a route to a
// prefix. The router-id and next hop come from the preceding RouterId/NextHop
// TLVs in the same packet. This port always sends full prefixes (omitted = 0).
//
class BabelUpdateTlv extends BabelTlv
{
    tlvType = BABEL_UPDATE;
    uint8_t ae = 0;
    uint8_t flags = 0;
    uint16_t interval = 0; // centiseconds
    uint16_t seqno = 0;
    uint16_t metric = 0;
    L3Address prefix; // the masked prefix address
    uint8_t prefixLen = 0;
}

//
// Route Request (type 9): asks a neighbour to send an Update for a prefix
// (wildcard AE with prefixLen 0 requests a full route dump).
//
class BabelRouteReqTlv extends BabelTlv
{
    tlvType = BABEL_ROUTEREQ;
    uint8_t ae = 0;
    L3Address prefix;
    uint8_t prefixLen = 0;
}

//
// Seqno Request (type 10): asks for an Update with a seqno greater than the
// given one, for loop-free recovery.
//
class BabelSeqnoReqTlv extends BabelTlv
{
    tlvType = BABEL_SEQNOREQ;
    uint8_t ae = 0;
    uint16_t seqno = 0;
    uint8_t hopcount = 0;
    uint8_t reserved = 0;
    uint32_t routerIdHi = 0;
    uint32_t routerIdLo = 0;
    L3Address prefix;
    uint8_t prefixLen = 0;
}
