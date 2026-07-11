//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.rsvpte.RsvpPacket;

namespace inet;

//
// RFC 2961 Section 5.3 Srefresh message (Workstream C8, refresh reduction).
//
// Sent instead of a full Path/Resv refresh once the corresponding PSB/RSB has
// already had a MESSAGE_ID object (~RsvpPacket's hasMessageId/messageId fields)
// assigned and sent at least once: the receiver, recognizing the id, treats this
// as equivalent to a full refresh of that state (RsvpTe.cc's processSrefreshMsg())
// without the full object set being retransmitted.
//
// Model simplification: RFC 2961 allows one Srefresh to list a whole batch of
// MESSAGE_IDs (one per refreshed state); this model always sends exactly one id
// per Srefresh (one per PSB, or one per (RSB, previous-hop) pair -- see
// RsvpTe::sendPathSrefresh()/sendResvSrefresh()), so no session/sender identity
// travels on this message at all: the receiver resolves which local PSB/RSB is
// meant purely from (sending peer address, epoch, message id).
class RsvpSrefreshMsg extends RsvpMessage
{
    uint32_t messageIdEpoch;
    uint32_t messageId;

    // Same MESSAGE_ID_ACK/_NACK piggyback convention as RsvpPacket's fields (see
    // there for the full rationale); duplicated here because RsvpSrefreshMsg does
    // not extend RsvpPacket (it carries no SESSION object).
    bool hasMessageIdAck = false;
    bool messageIdNack = false;
    uint32_t ackedMessageIdEpoch = 0;
    uint32_t ackedMessageId = 0;

    rsvpKind = SREFRESH_MESSAGE;
}
