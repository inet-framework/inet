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
