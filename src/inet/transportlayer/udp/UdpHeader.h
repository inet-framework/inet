//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_UDPPACKET_H
#define __INET_UDPPACKET_H

#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

class INET_API UdpHeader : public UdpHeader_Base, public ITransportPacket
{
  private:
    void copy(const UdpHeader& other) {}

  public:
    UdpHeader() : UdpHeader_Base() {}
    UdpHeader(const UdpHeader& other) : UdpHeader_Base(other) {copy(other);}
    UdpHeader& operator=(const UdpHeader& other) {if (this==&other) return *this; UdpHeader_Base::operator=(other); copy(other); return *this;}

    virtual UdpHeader *dup() const override { return new UdpHeader(*this); }

    /**
     * getter/setter for totalLength field of UDP packet
     * if set to -1, then getter returns getByteLength()
     */
    //int getTotalLengthField() const override { if (totalLengthField == -1) throw cRuntimeError("invalid totalLength field value=-1 in UDP header"); return totalLengthField; }

    virtual unsigned int getSourcePort() const override { return UdpHeader_Base::getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { UdpHeader_Base::setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return UdpHeader_Base::getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { UdpHeader_Base::setDestPort(port); }
};

} // namespace inet

#endif // ifndef __INET_TCPSEGMENT_H

