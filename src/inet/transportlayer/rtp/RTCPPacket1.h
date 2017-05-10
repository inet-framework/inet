//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_RTCPPACKET1_H
#define __INET_RTCPPACKET1_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/rtp/RTCPPacket1_m.h"

namespace inet {

namespace rtp {

class INET_API RTCPPacket : public RTCPPacket_Base
{
  public:
    RTCPPacket() : RTCPPacket_Base() {};
    RTCPPacket(const RTCPPacket& other) : RTCPPacket_Base(other) {}
    RTCPPacket& operator=(const RTCPPacket& other) { RTCPPacket_Base::operator=(other); return *this; }
    virtual RTCPPacket *dup() const override { return new RTCPPacket(*this); }

    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPPacket_Base

    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    virtual int getRtcpLength() const override { return (int)(byte(getChunkLength()).get() / 4) - 1; }
    virtual void setRtcpLength(int rtcpLength_var) override { throw cRuntimeError("Don't use setRtcpLength()!"); };
};

} // namespace rtp

} // namespace inet

#endif    // __INET_RTCPPACKET1_H_

