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

#ifndef __INET_RTCPPACKET2_H
#define __INET_RTCPPACKET2_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/rtp/RTCPPacket2_m.h"

namespace inet {

namespace rtp {

class INET_API RTCPReceiverReportPacket : public RTCPReceiverReportPacket_Base
{
  public:
    RTCPReceiverReportPacket(const char *name = nullptr, int kind = 0);
    RTCPReceiverReportPacket(const RTCPReceiverReportPacket& other) : RTCPReceiverReportPacket_Base(other) {}
    RTCPReceiverReportPacket& operator=(const RTCPReceiverReportPacket& other) { RTCPReceiverReportPacket_Base::operator=(other); return *this; }
    virtual RTCPReceiverReportPacket *dup() const override { return new RTCPReceiverReportPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPReceiverReportPacket_Base
    /**
     * Adds a receiver report to this receiver report packet.
     */
    virtual void addReceptionReport(ReceptionReport *report);
};

class INET_API RTCPSDESPacket : public RTCPSDESPacket_Base
{
  public:
    RTCPSDESPacket(const char *name = nullptr, int kind = 0);
    RTCPSDESPacket(const RTCPSDESPacket& other) : RTCPSDESPacket_Base(other) {}
    RTCPSDESPacket& operator=(const RTCPSDESPacket& other) { RTCPSDESPacket_Base::operator=(other); return *this; }
    virtual RTCPSDESPacket *dup() const override { return new RTCPSDESPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSDESPacket_Base
    void addSDESChunk(SDESChunk *sdesChunk);
};

class INET_API RTCPByePacket : public RTCPByePacket_Base
{
  public:
    RTCPByePacket(const char *name = nullptr, int kind = 0);
    RTCPByePacket(const RTCPByePacket& other) : RTCPByePacket_Base(other) {}
    RTCPByePacket& operator=(const RTCPByePacket& other) { RTCPByePacket_Base::operator=(other); return *this; }
    virtual RTCPByePacket *dup() const override { return new RTCPByePacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPByePacket_Base
};

} // namespace rtp

} // namespace inet

#endif    // __INET_RTCPPACKET2_H_

