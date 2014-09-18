//
// Copyright (C) 2011 Andras Varga
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

#ifndef __INET_IPV4DATAGRAM_H
#define __INET_IPV4DATAGRAM_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"

namespace inet {

/**
 * Represents an IPv4 datagram. More info in the IPv4Datagram.msg file
 * (and the documentation generated from it).
 */
class INET_API IPv4Datagram : public IPv4Datagram_Base, public INetworkDatagram
{
  public:
    IPv4Datagram(const char *name = NULL, int kind = 0) : IPv4Datagram_Base(name, kind) {}
    IPv4Datagram(const IPv4Datagram& other) : IPv4Datagram_Base(other) {}
    IPv4Datagram& operator=(const IPv4Datagram& other) { IPv4Datagram_Base::operator=(other); return *this; }

    virtual IPv4Datagram *dup() const { return new IPv4Datagram(*this); }

    /**
     * Returns bits 0-5 of the Type of Service field, a value in the 0..63 range
     */
    virtual int getDiffServCodePoint() const { return getTypeOfService() & 0x3f; }

    /**
     * Sets bits 0-5 of the Type of Service field; expects a value in the 0..63 range
     */
    virtual void setDiffServCodePoint(int dscp) { setTypeOfService((getTypeOfService() & 0xc0) | (dscp & 0x3f)); }

    /**
     * Returns bits 6-7 of the Type of Service field, a value in the range 0..3
     */
    virtual int getExplicitCongestionNotification() const { return (getTypeOfService() >> 6) & 0x03; }

    /**
     * Sets bits 6-7 of the Type of Service; expects a value in the 0..3 range
     */
    virtual void setExplicitCongestionNotification(int ecn) { setTypeOfService((getTypeOfService() & 0x3f) | ((ecn & 0x3) << 6)); }

    virtual L3Address getSourceAddress() const { return L3Address(getSrcAddress()); }
    virtual void setSourceAddress(const L3Address& address) { setSrcAddress(address.toIPv4()); }
    virtual L3Address getDestinationAddress() const { return L3Address(getDestAddress()); }
    virtual void setDestinationAddress(const L3Address& address) { setDestAddress(address.toIPv4()); }
    virtual int getTransportProtocol() const { return IPv4Datagram_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) { IPv4Datagram_Base::setTransportProtocol(protocol); }
};

} // namespace inet

#endif // ifndef __INET_IPV4DATAGRAM_H

