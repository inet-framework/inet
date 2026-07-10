//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

//
// LDP message types
//
enum LdpMessageTypes
{
    NOTIFICATION = 10;
    HELLO = 11;
    INITIALIZATION = 12;
    KEEP_ALIVE = 13;
    ADDRESS = 14;
    ADDRESS_WITHDRAW = 15;
    LABEL_MAPPING = 16;
    LABEL_REQUEST = 17;
    LABEL_WITHDRAW = 18;
    LABEL_RELEASE = 19;
    UNKNOWN = 20;
}

// Status Code values (RFC 5036 Section 3.9 / IANA "Label Distribution Protocol (LDP)
// Parameters" registry, "Status Code Name Space"); only the subset actually used by
// this model is defined here.
enum LdpStatusTypes
{
    SHUTDOWN = 10;
    NO_ROUTE = 13;
    KEEPALIVE_TIMER_EXPIRED = 20; // RFC 5036 Section 2.5.6: no LDP message (Keepalive or otherwise) received from the peer within the negotiated KeepAlive Time; distinct from SESSION_REJECTED_BAD_KEEPALIVE_TIME, which is a negotiation-time rejection, not a runtime liveness failure
    SESSION_REJECTED_BAD_KEEPALIVE_TIME = 24;
}

struct FecTlv
{
    Ipv4Address addr;
    int length;
}

//
// Base class for LDP packets
//
// Model simplification (see Ldp.ned): this model always carries exactly one
// message per PDU, so a single class hierarchy carries both the RFC 5036
// Section 3.5.3 PDU common header and the Section 3.6 message header fields.
// The PDU length and message length fields are not stored here: they are
// derived from the chunk length by the serializer (see LdpPacketSerializer).
//
class LdpPacket extends FieldsChunk
{
    // PDU common header (RFC 5036 Section 3.5.3)
    uint16_t version = 1;
    Ipv4Address lsrId; // this PDU's originating LSR's router id (the on-wire equivalent of the old fictional senderAddress field)
    uint16_t labelSpace = 0;

    // message header (RFC 5036 Section 3.6); message type is the LdpMessageTypes enum value
    int type;
    uint32_t messageId = 0; // not populated by this model (no message using it is currently required to be unique); present for wire-faithful representation
}

//
// LDP Label Mapping Message
//
class LdpLabelMapping extends LdpPacket
{
    FecTlv fec;
    int label;
}

//
// LDP Label Request Message
//
class LdpLabelRequest extends LdpPacket
{
    FecTlv fec;
}

//
// LDP Hello Message
//
class LdpHello extends LdpPacket
{
    uint16_t holdTime; // seconds; Common Hello Parameters TLV (RFC 5036 Section 3.5.2) hold time field width
    bool tbit;
    bool rbit;
}

//
// LDP notification message
//
class LdpNotify extends LdpPacket
{
    int status;
    FecTlv fec;
}

//
// LDP Initialization Message
//
// Fields mirror the Common Session Parameters TLV (RFC 5036 Section 3.5.3).
// The receiver's LDP Identifier is itself a 6-byte (4-byte LSR-Id + 2-byte
// label space) value, hence the split receiverLsrId/receiverLabelSpace fields.
// Sent by both sides during session establishment (see Ldp::sendInit); the
// A-bit (label advertisement discipline) is currently always 0 (DU) -- it gets
// wired to the distributionMode parameter by a later commit in this workstream.
class LdpIni extends LdpPacket
{
    uint16_t keepAliveTime = 0;
    bool abit;
    bool dbit;
    uint16_t pvLim = 0;
    uint16_t maxPduLength = 0;
    Ipv4Address receiverLsrId;
    uint16_t receiverLabelSpace = 0;
}

//
// LDP KeepAlive Message
//
// RFC 5036 Section 3.5.4: no message parameters -- the message header (with
// type = KEEP_ALIVE) is the entire message.
class LdpKeepAlive extends LdpPacket
{
}

//
// LDP Address Message
//
//# FIXME currently unused
// Fields mirror the Address List TLV (RFC 5036 Section 3.4.2): an address
// family (1 = IPv4) followed by a list of addresses of that family.
class LdpAddress extends LdpPacket
{
    bool isWithdraw;
    uint16_t addressFamily = 1;
    Ipv4Address addresses[];
}
