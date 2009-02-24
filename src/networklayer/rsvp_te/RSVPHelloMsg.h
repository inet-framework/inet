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

#ifndef __INET_RSVPHELLOMSG_H
#define __INET_RSVPHELLOMSG_H

#include "RSVPHello_m.h"


/**
 * RSVP HELLO REQUEST message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */

// XXX FIXME all RSVP message have kind=RSVP_TRAFFIC
// to distinguish them:
// if they are RSVPPacket: use packet->getRsvpKind()
// if they are RSVPHelloMsg: only one type exists
//
// thus, we need dynamic_cast to find out what it is
// that's not good. we don't use kind, because kind
// is used by IP QoS mechanism (use DS field for IP QoS instead???)

class RSVPHelloMsg : public RSVPHelloMsg_Base
{
  public:
    RSVPHelloMsg(const char *name=NULL, int kind=RSVP_TRAFFIC) : RSVPHelloMsg_Base(name,kind) {}
    RSVPHelloMsg(const RSVPHelloMsg& other) : RSVPHelloMsg_Base(other.getName()) {operator=(other);}
    RSVPHelloMsg& operator=(const RSVPHelloMsg& other) {RSVPHelloMsg_Base::operator=(other); return *this;}
    virtual RSVPHelloMsg *dup() const {return new RSVPHelloMsg(*this);}
};

#endif



