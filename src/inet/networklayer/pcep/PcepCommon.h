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
