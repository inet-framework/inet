//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.NetworkHeaderBase;
import inet.networklayer.rsvpte.IntServ;

namespace inet;

enum RsvpConstants {
    PATH_MESSAGE   = 1;
    RESV_MESSAGE   = 2;
    PTEAR_MESSAGE  = 3;
    RTEAR_MESSAGE  = 4;
    PERROR_MESSAGE = 5;
    RERROR_MESSAGE = 6;
    HELLO_MESSAGE  = 7;
    SREFRESH_MESSAGE = 8; // RFC 2961 Section 5.3 Srefresh (Workstream C8); wire message type 15, see RsvpTeSerializer.cc's translation table
}

//
// RSVP message common part
//
class RsvpMessage extends FieldsChunk
{
    int rsvpKind;
}

//
// FIXME missing documentation
//
class RsvpPacket extends RsvpMessage
{
    SessionObj session;

    // RFC 2961 Section 5.4 refresh reduction (Workstream C8), gated on the sending
    // router's "refreshReduction" parameter (default off, so these fields default to
    // "absent" for every existing example/showcase). MESSAGE_ID identifies the
    // content of the PSB/RSB state this particular message carries, so that a later
    // Srefresh (~RsvpSrefreshMsg) naming the same id can stand in for a full resend of
    // it. Presence on the wire is inferred from this message's own recorded total
    // length (RsvpTeSerializer's existing ERO/RRO trailing-optional-object
    // convention), not a dedicated flag byte.
    bool hasMessageId = false;
    uint32_t messageIdEpoch = 0;
    uint32_t messageId = 0;

    // RFC 2961 Section 5.4 MESSAGE_ID_ACK / MESSAGE_ID_NACK object. Workstream C8
    // scope choice: acknowledgments are piggybacked only, on whatever message this
    // router next sends back toward the id's originator (Path/Resv/Srefresh) --
    // there is no standalone Ack message type in this model. A set "nack" means the
    // id named below was NOT recognized by this router (see RsvpTe.cc's
    // processSrefreshMsg()); the originator falls back to a full resend on
    // receiving it.
    bool hasMessageIdAck = false;
    bool messageIdNack = false;
    uint32_t ackedMessageIdEpoch = 0;
    uint32_t ackedMessageId = 0;
}

cplusplus(RsvpPacket) {{

/**
 * RsvpPacket common part.
 *
 * Adds convenience get() and set() methods to the generated class, but no extra data.
 */
  public:
    inline Ipv4Address getDestAddress() { return getSession().DestAddress; }
    inline int getTunnelId() { return getSession().Tunnel_Id; }
    inline int getExTunnelId() { return getSession().Extended_Tunnel_Id; }
    inline int getSetupPri() { return getSession().setupPri; }
    inline int getHoldingPri() { return getSession().holdingPri; }
    inline bool isInSession(SessionObj *s) const
    {
        return getSession().DestAddress == s->DestAddress &&
               getSession().Tunnel_Id == s->Tunnel_Id &&
               getSession().Extended_Tunnel_Id == s->Extended_Tunnel_Id;
    }

}}
