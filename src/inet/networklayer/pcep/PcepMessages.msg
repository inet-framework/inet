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
// Header). Phase 1 (session establishment) defined OPEN/KEEPALIVE; Phase 2
// (stateless path computation) added PCREQ/PCREP; Phase 3 (RFC 8231 stateful
// delegation, Workstream F4) adds PCRPT/PCUPD.
//
enum PcepMessageType
{
    PCEP_OPEN = 1;
    PCEP_KEEPALIVE = 2;
    PCEP_PCREQ = 3;
    PCEP_PCREP = 4;
    PCEP_PCRPT = 10; // RFC 8231 Section 6.1's own Message-Type value (numbering is not contiguous with RFC 5440's)
    PCEP_PCUPD = 11; // RFC 8231 Section 6.2's own Message-Type value
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

//
// PCEP Path Computation State Report (PCRpt) Message (RFC 8231 Section 6.1), sent by
// a PCC to report a delegated, PCC-managed (~RsvpTe ingress) LSP's state to the PCE
// (Workstream F4 Phase 3 -- stateful delegation). Sent whenever such an LSP comes up
// (including a make-before-break cutover onto a new route) or is torn down; carries:
// - an SRP object (Section 7.2): model simplification, only the SRP-ID-number is
//   kept (mirrors the RP object's own precedent in PcepPcreq above) -- correlates
//   this report with the last PCUpd it is acknowledging; 0 means unsolicited (this
//   report was not triggered by any PCUpd).
// - an LSP object (Section 7.3): model simplification -- only the PLSP-ID (a small
//   in-model integer ~RsvpTe assigns once per delegated LSP and both sides use for
//   its whole lifetime, stable across an internal make-before-break re-route) plus
//   two of the object's flag bits are kept: "delegate" (the D flag: this PCC
//   delegates control of this LSP to the PCE -- this model's PCC only ever reports
//   delegate=true, see the field's own doc comment) and "up" (a boolean collapse of
//   the RFC's finer-grained Operational-Status field (DOWN/GOING-UP/UP/GOING-DOWN)
//   -- a documented model simplification).
// - END-POINTS/BANDWIDTH/LSPA objects, verbatim in shape to PcepPcreq's own fields
//   above: NOT part of the RFC 8231 baseline PCRpt, but legitimately allowed as
//   optional <path> attributes (RFC 8231 Section 6.1's grammar) -- always included
//   here (rather than conditionally) so ~Pce::reoptimizeDelegatedLsp() has the same
//   CSPF inputs (source/dest/bandwidth/priority/affinity) the original PCReq (or
//   the operator's static config) supplied, without needing to separately remember
//   Phase 2's already-consumed, ephemeral per-request cache.
// - an ERO object (Section 7.9, reusing ~EroVector verbatim, as PcepPcrep does):
//   the path currently in use; meaningful only when up == true.
//
class PcepPcrpt extends PcepMessage
{
    uint32_t srpId = 0; // SRP object: SRP-ID-number correlating this report to the PCUpd it acknowledges; 0 = unsolicited
    int plspId; // LSP object: PLSP-ID identifying this LSP for the lifetime of the delegation
    bool delegate = true; // LSP object: D (Delegate) flag; always true in this model (see class doc comment -- there is no partial/returned-delegation state modeled)
    bool up = false; // LSP object: model-simplified boolean Operational-Status; true: this ingress LSP is up and using `ero`; false: it has been torn down (`ero`/the CSPF-input fields below are meaningless)
    Ipv4Address srcAddress; // see class doc comment: this LSP's CSPF root (the reporting PCC's own router id)
    Ipv4Address dstAddress;
    double bandwidth;
    int setupPriority;
    uint32_t includeAny = 0;
    uint32_t excludeAny = 0;
    EroVector ero; // ERO object: the path currently in use; meaningful iff up
}

//
// PCEP Path Computation Update (PCUpd) Message (RFC 8231 Section 6.2), sent by the
// PCE to update a delegated LSP's route (Workstream F4 Phase 3). Carries:
// - an SRP object (Section 7.2): this update's OWN correlator (model-assigned by
//   the PCE), echoed by the PCC's next PCRpt for this LSP (PcepPcrpt::srpId above).
// - an LSP object (Section 7.3): the PLSP-ID naming which delegated LSP to update
//   (no flags are meaningful here: a PCUpd only ever targets an already-delegated
//   LSP, so D is implicitly true and O is moot -- the PCC is the one reporting
//   operational status, not the PCE).
// - an ERO object (Section 7.9, reusing ~EroVector verbatim): the new route to
//   apply. Unlike PcepPcrpt above, no END-POINTS/BANDWIDTH/LSPA are carried: this
//   model's PCE never changes a delegated LSP's bandwidth/priority/affinity via a
//   PCUpd, only its route.
//
class PcepPcupd extends PcepMessage
{
    uint32_t srpId; // SRP object: this update's own correlator, echoed by the PCC's next PCRpt for this LSP
    int plspId; // LSP object: PLSP-ID naming which delegated LSP to update
    EroVector ero; // ERO object: the new route to apply
}
