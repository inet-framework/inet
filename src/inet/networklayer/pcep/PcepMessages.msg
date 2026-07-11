//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.rsvpte.IntServ;

namespace inet;

//
// PCEP message types (RFC 5440 Section 6.1, Message-Type field of the Common
// Header). Phase 1 (session establishment) defined OPEN/KEEPALIVE; Phase 2 (this
// workstream's stateless path computation) adds PCREQ/PCREP. PCRpt/PCUpd etc. (RFC
// 8231 stateful delegation) are left for a later phase.
//
enum PcepMessageType
{
    PCEP_OPEN = 1;
    PCEP_KEEPALIVE = 2;
    PCEP_PCREQ = 3;
    PCEP_PCREP = 4;
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

//
// PCEP Path Computation Request (PCReq) Message (RFC 5440 Section 6.5), sent by a
// PCC to ask the PCE to compute a path (Workstream F4 Phase 2 -- stateless path
// computation). Carries:
// - an RP (Request Parameters) object (Section 7.4): model simplification --
//   only the Request-ID-number is kept (to correlate this request with its
//   PCRep, see ~Pcc), the RFC's flag bits (e.g. the reoptimization (R) bit) are
//   not modeled since this phase never reoptimizes via PCEP (that is Phase 3's
//   PCRpt/PCUpd delegation, out of scope here).
// - an END-POINTS object (Section 7.6, IPv4 C-Type): source + destination
//   addresses -- this model's PCEP stays IPv4-only, like the rest of the MPLS/TE
//   stack it serves.
// - a BANDWIDTH object (Section 7.7): the requested bandwidth, mirroring
//   ~RsvpTe's own TrafficPath::tspec.req_bandwidth -- the same input the PCE's
//   ~Ted::calculateShortestPath() CSPF needs to prune infeasible links exactly
//   like ~RsvpTe's own local CSPF call does (Workstream C6).
// - an LSPA object (Section 7.11): setup priority (mirroring
//   ~SessionObj::setupPri) plus, opportunistically, the same object's
//   Include-any/Exclude-any fields for optional CSPF affinity constraints
//   (Workstream D3-style, 0 = no constraint) -- convenient to carry here since
//   LSPA already has dedicated fields for them; holding priority/flags are not
//   modeled (this phase's PCE never preempts).
//
class PcepPcreq extends PcepMessage
{
    uint32_t requestId; // RP object: Request-ID-number, model-assigned by the requesting Pcc to correlate this PCReq with the matching PCRep
    Ipv4Address srcAddress; // END-POINTS object: source address (the requesting PCC's own router id -- the CSPF root)
    Ipv4Address dstAddress; // END-POINTS object: destination address
    double bandwidth; // BANDWIDTH object: requested bandwidth (bits/sec), mirrors TrafficPath::tspec.req_bandwidth
    int setupPriority; // LSPA object: setup priority, mirrors SessionObj::setupPri
    uint32_t includeAny = 0; // LSPA object: include-any affinity mask (D3-style), 0 = no constraint
    uint32_t excludeAny = 0; // LSPA object: exclude-any affinity mask, 0 = no constraint
}

//
// PCEP Path Computation Reply (PCRep) Message (RFC 5440 Section 6.6): the PCE's
// answer to a PCReq. Always carries the RP object (echoing the request's
// Request-ID-number, the correlator back to the waiting Pcc), plus EITHER:
// - an ERO object (Section 7.9) on success -- reuses ~EroVector/~EroObj
//   (~IntServ.msg) verbatim rather than redefining them: RFC 5440's ERO is the
//   same object (same IPv4-prefix-subobject wire format, RFC 3209 Section
//   4.3.3.1) that ~RsvpTe/~RsvpPathMsg already carry, just computed remotely
//   instead of locally; or
// - a NO-PATH object (Section 7.5) on failure -- model simplification: a single
//   presence flag is enough (noPath=true), the RFC's optional unreachable-
//   destination sub-TLVs are not modeled.
//
class PcepPcrep extends PcepMessage
{
    uint32_t requestId; // RP object: echoes the matching PCReq's Request-ID-number
    bool noPath = false; // true: NO-PATH object present (ero is meaningless); false: ERO object present (see ero)
    EroVector ero; // ERO object; meaningful iff !noPath
}
