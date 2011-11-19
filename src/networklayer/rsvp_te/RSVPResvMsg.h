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


/**
 * RSVP RESV message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class RSVPResvMsg : public RSVPResvMsg_Base
{
  public:
    RSVPResvMsg(const char *name = NULL, int kind = RESV_MESSAGE) : RSVPResvMsg_Base(name, kind) {}
    RSVPResvMsg(const RSVPResvMsg& other) : RSVPResvMsg_Base(other) {}
    RSVPResvMsg& operator=(const RSVPResvMsg& other) {RSVPResvMsg_Base::operator=(other); return *this;}
    virtual RSVPResvMsg *dup() const {return new RSVPResvMsg(*this);}

    inline IPv4Address getNHOP() {return getHop().Next_Hop_Address;}
    inline IPv4Address getLIH() {return getHop().Logical_Interface_Handle;}
};


/**
 * RESV TEAR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class RSVPResvTear : public RSVPResvTear_Base
{
  public:
    RSVPResvTear(const char *name = NULL, int kind = RTEAR_MESSAGE) : RSVPResvTear_Base(name, kind) {}
    RSVPResvTear(const RSVPResvTear& other) : RSVPResvTear_Base(other) {}
    RSVPResvTear& operator=(const RSVPResvTear& other) {RSVPResvTear_Base::operator=(other); return *this;}
    virtual RSVPResvTear *dup() const {return new RSVPResvTear(*this);}

    inline IPv4Address getNHOP() {return getHop().Next_Hop_Address;}
    inline IPv4Address getLIH() {return getHop().Logical_Interface_Handle;}
};


/**
 * RESV ERROR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class RSVPResvError : public RSVPResvError_Base
{
  public:
    RSVPResvError(const char *name = NULL, int kind = RERROR_MESSAGE) : RSVPResvError_Base(name, kind) {}
    RSVPResvError(const RSVPResvError& other) : RSVPResvError_Base(other) {}
    RSVPResvError& operator=(const RSVPResvError& other) {RSVPResvError_Base::operator=(other); return *this;}
    virtual RSVPResvError *dup() const {return new RSVPResvError(*this);}

    inline IPv4Address getNHOP() {return getHop().Next_Hop_Address;}
    inline IPv4Address getLIH() {return getHop().Logical_Interface_Handle;}
};

#endif



