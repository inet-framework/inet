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

cplusplus {{
#define RSVP_TRAFFIC    2
}}

enum RsvpConstants {
    PATH_MESSAGE   = 1;
    RESV_MESSAGE   = 2;
    PTEAR_MESSAGE  = 3;
    RTEAR_MESSAGE  = 4;
    PERROR_MESSAGE = 5;
    RERROR_MESSAGE = 6;
    HELLO_MESSAGE  = 7;
}

//
// RSVP message common part.
//
// NOTE: these RSVP/RSVP-TE message classes are a simulation model, not the
// on-the-wire RSVP format, and there is deliberately no RSVP serializer.
// They carry only what INET's RSVP-TE signaling needs -- e.g. RsvpPathMsg holds
// a hop, a sender descriptor, a label_request, an ERO and an INET-internal color
// and rsvpKind. A real RSVP message (RFC 2205 / RFC 3209 RSVP-TE) is instead a
// common header followed by a sequence of type-length objects (SESSION, RSVP_HOP,
// TIME_VALUES, SENDER_TEMPLATE, SENDER_TSPEC, ADSPEC, LABEL_REQUEST,
// EXPLICIT_ROUTE, RECORD_ROUTE, ...), which do not correspond one-to-one to these
// fields: the model lacks objects that appear on the wire (ADSPEC, TIME_VALUES,
// the real SESSION/HOP/TSPEC encodings) and adds fields that never do (color,
// rsvpKind, and the RSVP-TE label_request/ERO for plain RSVP). Serializing a
// captured RSVP message into this model would therefore be both lossy and large,
// and INET never serializes RSVP in normal simulations. A pcap capture that
// contains RSVP (e.g. mpls-basic.cap) consequently cannot round-trip and its run
// is disabled in serializer_pcap.test.
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
    bool checksumValid = true;
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
