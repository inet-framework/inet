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

#ifndef __INET_WiseRouteHeader_H
#define __INET_WiseRouteHeader_H

#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/networklayer/wiseroute/WiseRouteHeader_m.h"

namespace inet {

/**
 * Represents an WiseRoute datagram. More info in the WiseRouteHeader.msg file
 * (and the documentation generated from it).
 */
class INET_API WiseRouteHeader : public WiseRouteHeader_Base, public INetworkDatagram
{
  public:
    WiseRouteHeader() : WiseRouteHeader_Base() {}
    WiseRouteHeader(const WiseRouteHeader& other) : WiseRouteHeader_Base(other) {}
    WiseRouteHeader& operator=(const WiseRouteHeader& other) { WiseRouteHeader_Base::operator=(other); return *this; }

    virtual WiseRouteHeader *dup() const override { return new WiseRouteHeader(*this); }

    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address.toModuleId()); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address.toModuleId()); }
    virtual int getTransportProtocol() const override { return WiseRouteHeader_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) override { WiseRouteHeader_Base::setTransportProtocol(protocol); };
};

} // namespace inet

#endif // ifndef __INET_WiseRouteHeader_H

