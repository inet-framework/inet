//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet::lisp;

enum LispMsgType
{
    LISP_REQUEST = 1;
    LISP_REPLY = 2;
    LISP_REGISTER = 3;
    LISP_NOTIFY = 4;
    LISP_ENCAPSULATED_REQUEST = 8;
    LISP_CACHE_SYNC = 6;
    LISP_CACHE_SYNC_ACK = 7;
}

//
// An AFI-encoded address (the L3Address carries its own address family).
//
struct LispAfiAddr
{
    @packetData;
    L3Address address;
}

//
// A single locator (RLOC) inside a Map-Reply/Register record.
//
struct LispLocatorRecord
{
    @packetData;
    uint8_t priority;
    uint8_t weight;
    uint8_t mpriority;
    uint8_t mweight;
    bool localLocBit;   // L: locator belongs to the sender
    bool probedBit;     // p: RLOC-probe reply
    bool reachableBit;  // R: locator is reachable
    L3Address rloc;
}

//
// One EID-to-RLOC mapping record (Map-Reply / Map-Register / Map-Notify).
// A class (not a struct) because it carries a dynamic locator array.
//
class LispMapRecord
{
    @packetData;
    uint32_t recordTTL;          // minutes
    uint8_t eidMaskLength;
    uint8_t act;                 // ~LispCommon::EAct
    bool aBit;                   // Authoritative
    uint16_t mapVersion;
    L3Address eidPrefix;
    LispLocatorRecord locators[];
}

//
// A queried EID prefix inside a Map-Request.
//
struct LispEidRecord
{
    @packetData;
    uint8_t eidMaskLength;
    L3Address eidPrefix;
}

//
// LISP data-plane header (RFC 6830, section 5.3), prepended to encapsulated traffic.
//
class LispHeader extends FieldsChunk
{
    chunkLength = B(8);
    bool nonceBit;          // N
    bool locStatBit;        // L
    bool echoNonceBit;      // E
    bool verMapBit;         // V
    bool instanceBit;       // I
    uint8_t flags;
    uint32_t nonce;             // nonce or (with V) map-version
    uint32_t instanceId;        // instance id or locator-status-bits
    uint32_t locatorStatusBits;
}

//
// Common base for the LISP control messages (RFC 6830, section 6).
//
class LispControlMessage extends FieldsChunk
{
    chunkLength = B(12);
    uint8_t type;          // ~LispMsgType
    uint64_t nonce;
    uint8_t recordCount;
}

//
// Map-Request (Type 1).
//
class LispMapRequest extends LispControlMessage
{
    type = LISP_REQUEST;
    bool aBit;             // Authoritative
    bool mapDataBit;       // Map-Data-Present
    bool probeBit;         // Probe
    bool smrBit;           // Solicit-Map-Request
    bool pitrBit;          // PITR
    bool smrInvokedBit;    // SMR-invoked
    uint8_t itrRlocCount;
    LispAfiAddr sourceEid;
    LispAfiAddr itrRlocs[];
    LispEidRecord recs[];
    LispMapRecord mapReply;    // optional Map-Reply record carrying the sender's own mapping
}

//
// Map-Reply (Type 2).
//
class LispMapReply extends LispControlMessage
{
    type = LISP_REPLY;
    bool probeBit;
    bool echoNonceBit;
    bool secBit;
    LispMapRecord records[];
}

//
// Map-Register (Type 3).
//
class LispMapRegister extends LispControlMessage
{
    type = LISP_REGISTER;
    bool proxyBit;
    bool mapNotifyBit;
    uint8_t keyId;
    uint16_t authDataLen;
    string authData;
    LispMapRecord records[];
}

//
// Map-Notify (Type 4) — Map-Server acknowledgement of a registration.
//
class LispMapNotify extends LispMapRegister
{
    type = LISP_NOTIFY;
}
