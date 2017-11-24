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

#ifndef __INET_RSVPRESVMSG_H
#define __INET_RSVPRESVMSG_H

#include "RSVPResvMsg_m.h"

namespace inet {

/**
 * RSVP RESV message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RsvpResvMsg : public RsvpResvMsg_Base
{
  public:
    RsvpResvMsg() : RsvpResvMsg_Base() {}
    RsvpResvMsg(const RsvpResvMsg& other) : RsvpResvMsg_Base(other) {}
    RsvpResvMsg& operator=(const RsvpResvMsg& other) { RsvpResvMsg_Base::operator=(other); return *this; }
    virtual RsvpResvMsg *dup() const override { return new RsvpResvMsg(*this); }

    inline Ipv4Address getNHOP() const { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() const { return getHop().Logical_Interface_Handle; }
};

/**
 * RESV TEAR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RsvpResvTear : public RsvpResvTear_Base
{
  public:
    RsvpResvTear(/* const char *name = nullptr, int kind = RTEAR_MESSAGE */) : RsvpResvTear_Base(/* name, kind */) {}
    RsvpResvTear(const RsvpResvTear& other) : RsvpResvTear_Base(other) {}
    RsvpResvTear& operator=(const RsvpResvTear& other) { RsvpResvTear_Base::operator=(other); return *this; }
    virtual RsvpResvTear *dup() const override { return new RsvpResvTear(*this); }

    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
};

/**
 * RESV ERROR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RsvpResvError : public RsvpResvError_Base
{
  public:
    RsvpResvError(/* const char *name = nullptr, int kind = RERROR_MESSAGE */) : RsvpResvError_Base(/* name, kind */) {}
    RsvpResvError(const RsvpResvError& other) : RsvpResvError_Base(other) {}
    RsvpResvError& operator=(const RsvpResvError& other) { RsvpResvError_Base::operator=(other); return *this; }
    virtual RsvpResvError *dup() const override { return new RsvpResvError(*this); }

    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
};

} // namespace inet

#endif // ifndef __INET_RSVPRESVMSG_H

