/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __RSVPPACKET_H__
#define __RSVPPACKET_H__

#include "RSVPPacket_m.h"

/**
 * RSVP message common part.
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPPacket: public RSVPPacket_Base
{
  public:
    RSVPPacket(const char *name=NULL, int kind=0) : RSVPPacket_Base(name,kind) {}
    RSVPPacket(const RSVPPacket& other) : RSVPPacket_Base(other.name()) {operator=(other);}
    RSVPPacket& operator=(const RSVPPacket& other) {RSVPPacket_Base::operator=(other); return *this;}
#if OMNETPP_VERSION<0x0302
    virtual RSVPPacket *dup() {return new RSVPPacket(*this);}
#else
    virtual RSVPPacket *dup() {return new RSVPPacket(*this);}
#endif
    inline IPADDR getDestAddress() {return getSession().DestAddress;}
    inline int getProtId()      {return getSession().Protocol_Id;}
    inline int getDestPort()    {return getSession().DestPort;}
    inline int getTunnelId()    {return getSession().Tunnel_Id;}
    inline int getExTunnelId()  {return getSession().Extended_Tunnel_Id;}
    inline int getSetupPri()    {return getSession().setupPri;}
    inline int getHoldingPri()  {return getSession().holdingPri;}
    inline bool isInSession(SessionObj_t* s) {
        return getSession().DestAddress==s->DestAddress &&
               getSession().DestPort==s->DestPort &&
               getSession().Protocol_Id==s->Protocol_Id;
    }
};

#endif
