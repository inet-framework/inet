//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_RSVPPACKET_H
#define __INET_RSVPPACKET_H

#include "RSVPPacket_m.h"

#define RSVP_TRAFFIC        2

/**
 * RSVP message common part.
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class RSVPPacket : public RSVPPacket_Base
{
  public:
    RSVPPacket(const char *name = NULL, int kind = 0) : RSVPPacket_Base(name, RSVP_TRAFFIC) { this->rsvpKind_var = kind; }
    RSVPPacket(const RSVPPacket& other) : RSVPPacket_Base(other) {}
    RSVPPacket& operator=(const RSVPPacket& other) {RSVPPacket_Base::operator=(other); return *this;}
    virtual RSVPPacket *dup() const {return new RSVPPacket(*this);}

    inline IPv4Address getDestAddress() {return getSession().DestAddress;}
    inline int getTunnelId()    {return getSession().Tunnel_Id;}
    inline int getExTunnelId()  {return getSession().Extended_Tunnel_Id;}
    inline int getSetupPri()    {return getSession().setupPri;}
    inline int getHoldingPri()  {return getSession().holdingPri;}
    inline bool isInSession(SessionObj_t* s) {
        return getSession().DestAddress==s->DestAddress &&
               getSession().Tunnel_Id==s->Tunnel_Id &&
               getSession().Extended_Tunnel_Id==s->Extended_Tunnel_Id;
    }
};

#endif
