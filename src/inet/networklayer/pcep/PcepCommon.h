//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCEPCOMMON_H
#define __INET_PCEPCOMMON_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units_m.h"

namespace inet {

// RFC 5440 Section 4.1: PCEP uses TCP port 4189, allocated by IANA.
#define PCEP_PORT 4189

// RFC 5440 Section 6.1: Common Header -- Version(3 bits)+Flags(5 bits, unused)
// +Message-Type(8 bits)+Message-Length(16 bits, INCLUSIVE of this header). 4 bytes total.
const B PCEP_COMMON_HEADER_BYTES = B(4);

// RFC 5440 Section 7.2: common object header -- Object-Class(8 bits)+Object-Type(4
// bits)/Res(2 bits)/P(1 bit)/I(1 bit)+Object-Length(16 bits, INCLUSIVE of this
// header) -- 4 bytes; the OPEN object body (RFC 5440 Section 7.3: Ver/Flags(1
// byte)+Keepalive(1 byte)+DeadTimer(1 byte)+SID(1 byte)) is another 4 bytes -- no
// TLVs modeled yet (see PcepMessages.msg / Pce.ned / Pcc.ned).
const B PCEP_OPEN_OBJECT_BYTES = B(8);
const B PCEP_OPEN_MESSAGE_BYTES = PCEP_COMMON_HEADER_BYTES + PCEP_OPEN_OBJECT_BYTES; // 12 B

// RFC 5440 Section 6.4: the Keepalive message is the Common Header alone, no object.
const B PCEP_KEEPALIVE_MESSAGE_BYTES = PCEP_COMMON_HEADER_BYTES; // 4 B

// Phase 2 (Workstream F4): PCReq/PCRep object sizes (RFC 5440 Section 7), all
// INCLUSIVE of their own 4-byte common object header (Section 7.2), same
// convention as PCEP_OPEN_OBJECT_BYTES above.

// RFC 5440 Section 7.4: RP (Request Parameters) object. Model simplification
// (mirrors the OPEN object's own precedent of dropping unused flag bits): the
// body carries only the Request-ID-number: this model never sets the RFC's flag
// bits (e.g. reoptimization (R)) since PCReq/PCRep here is always a single,
// stateless, non-reoptimizing request (see PcepMessages.msg).
const B PCEP_RP_OBJECT_BYTES = B(8); // 4-byte header + 4-byte Request-ID-number

// RFC 5440 Section 7.6: END-POINTS object (IPv4 C-Type): source + destination addresses.
const B PCEP_ENDPOINTS_OBJECT_BYTES = B(12); // 4-byte header + 4+4-byte addresses

// RFC 5440 Section 7.7: BANDWIDTH object (Requested Bandwidth C-Type): a single
// IEEE-754 float, matching ~RsvpTeSerializer's own bandwidth-field encoding.
const B PCEP_BANDWIDTH_OBJECT_BYTES = B(8); // 4-byte header + 4-byte float

// RFC 5440 Section 7.11: LSPA (LSP Attributes) object. Model simplification: only
// Exclude-any/Include-any (D3-style CSPF affinity) and Setup Priority are
// meaningful; Holding Priority/Flags/Reserved are written as 0 and ignored on
// read (this model's PCE never preempts).
const B PCEP_LSPA_OBJECT_BYTES = B(16); // 4-byte header + 4+4 (exclude/include-any) + 1+1+1+1 (setup pri/holding pri/flags/reserved)

// RFC 5440 Section 6.5: PCReq = Common Header + RP + END-POINTS + BANDWIDTH + LSPA
// -- always this same fixed size (no ERO in a request).
const B PCEP_PCREQ_MESSAGE_BYTES = PCEP_COMMON_HEADER_BYTES + PCEP_RP_OBJECT_BYTES
    + PCEP_ENDPOINTS_OBJECT_BYTES + PCEP_BANDWIDTH_OBJECT_BYTES + PCEP_LSPA_OBJECT_BYTES; // 48 B

// RFC 5440 Section 7.5: NO-PATH object. Model simplification: no unreachable-
// destination sub-TLVs -- the object's mere presence signals "no path" (its
// Nature-of-Issue/Flags/Reserved body is written/ignored as 0).
const B PCEP_NOPATH_OBJECT_BYTES = B(8); // 4-byte header + 4-byte body (unused)

// RFC 5440 Section 6.6: PCRep = Common Header + RP + (ERO | NO-PATH). The
// NO-PATH variant is this fixed size; the ERO variant is variable (RFC 3209
// Section 4.3.3.1 IPv4-prefix-subobject format, reused verbatim by RFC 5440
// Section 7.9: a 4-byte ERO object header + 8 bytes per hop) so there is no
// single constant for it -- see pcepPcrepEroMessageBytes() below.
const B PCEP_PCREP_NOPATH_MESSAGE_BYTES = PCEP_COMMON_HEADER_BYTES + PCEP_RP_OBJECT_BYTES + PCEP_NOPATH_OBJECT_BYTES; // 20 B

// Convenience for callers building a successful PCRep (~Pce) that need to set the
// message's chunkLength before inserting it into a Packet, mirroring how OPEN's
// fixed PCEP_OPEN_MESSAGE_BYTES is used directly for that same purpose.
inline B pcepPcrepEroMessageBytes(size_t eroHopCount)
{
    return PCEP_COMMON_HEADER_BYTES + PCEP_RP_OBJECT_BYTES + B(4 + 8 * eroHopCount);
}

// PCEP session FSM (RFC 5440 Section 6.2, minimal subset -- mirrors the level of
// fidelity of Ldp::peer_info::SessionState, not the full RFC state machine):
// NONEXISTENT until the TCP connection is established; INITIALIZED once connected
// but before any Open has been sent/received; OPENSENT once we've sent our own Open
// (the Pcc only -- the Pce never sends an Open before it has received the Pcc's, since
// PCEP has no discovery phase and the Pcc always initiates); OPENREC once we've
// received the peer's Open and replied (with our own Open, if we hadn't already sent
// one, plus a Keepalive); OPERATIONAL once we've also received the peer's Keepalive
// acknowledging ours.
enum PcepSessionState { PCEP_NONEXISTENT, PCEP_INITIALIZED, PCEP_OPENSENT, PCEP_OPENREC, PCEP_OPERATIONAL };

inline const char *pcepSessionStateName(PcepSessionState state)
{
    switch (state) {
        case PCEP_NONEXISTENT: return "NONEXISTENT";
        case PCEP_INITIALIZED: return "INITIALIZED";
        case PCEP_OPENSENT: return "OPENSENT";
        case PCEP_OPENREC: return "OPENREC";
        case PCEP_OPERATIONAL: return "OPERATIONAL";
        default: return "???";
    }
}

} // namespace inet

#endif
