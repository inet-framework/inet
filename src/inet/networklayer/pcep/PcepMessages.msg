//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// PCEP message types (RFC 5440 Section 6.1, Message-Type field of the Common
// Header). Only the subset needed for Phase 1 of this workstream (session
// establishment) is defined here; PCReq/PCRep/PCRpt/PCUpd etc. are added by later
// phases (path computation, stateful delegation).
//
enum PcepMessageType
{
    PCEP_OPEN = 1;
    PCEP_KEEPALIVE = 2;
}

//
// Base class for PCEP messages (RFC 5440 Section 6.1: Common Header). Model
// simplification, mirroring ~LdpPacket: the Version/Message-Type fields live here
// as plain fields; the Flags field (unused by any message modeled so far) and
// Message-Length are not stored as msg fields -- Message-Length is derived from
// (and, on deserialize, used to reconstruct) the chunk length by
// ~PcepMessagesSerializer. Unlike LDP's PDU-length convention (which excludes the
// length field's own bytes), RFC 5440's Message-Length is INCLUSIVE of the 4-byte
// Common Header itself -- see PcepMessagesSerializer for the exact byte accounting.
//
class PcepMessage extends FieldsChunk
{
    uint8_t version = 1; // Common Header Version field (3 bits on the wire; see PcepMessagesSerializer)
    int type; // Message-Type (8 bits); PcepMessageType enum value
}

//
// PCEP Open Message (RFC 5440 Section 6.3), carrying exactly one OPEN object
// (Section 7.3): KeepAlive Timer + DeadTimer (each proposed by the sender) and a
// Session ID (SID) chosen by the sender. No optional TLVs (e.g. the Stateful PCE
// Capability TLV) are modeled yet -- those matter starting with this workstream's
// later stateful-delegation phase.
//
class PcepOpen extends PcepMessage
{
    uint8_t keepaliveTime; // OPEN object: KeepAlive Timer (seconds) proposed by the sender
    uint8_t deadTimer; // OPEN object: DeadTimer (seconds) proposed by the sender -- see ~Pce/~Pcc for how the RECEIVER uses this value (not its own configured deadTimer) to monitor the sender
    uint8_t sid; // OPEN object: Session ID (SID) chosen by the sender; a small in-model counter (see Pce::sidCounter/Pcc::sid), not otherwise consulted before stateful delegation (a later phase)
}

//
// PCEP Keepalive Message (RFC 5440 Section 6.4): the Common Header alone, no object.
//
class PcepKeepalive extends PcepMessage
{
}
