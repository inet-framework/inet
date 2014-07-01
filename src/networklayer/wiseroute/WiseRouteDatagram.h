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


#ifndef _WISEROUTEDATAGRAM_H_
#define _WISEROUTEDATAGRAM_H_

#include "INetworkDatagram.h"
#include "WiseRouteDatagram_m.h"

namespace inet {

/**
 * Represents an WiseRoute datagram. More info in the WiseRouteDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API WiseRouteDatagram : public WiseRouteDatagram_Base, public INetworkDatagram
{
  public:
    WiseRouteDatagram(const char *name = NULL, int kind = 0) : WiseRouteDatagram_Base(name, kind) {}
    WiseRouteDatagram(const WiseRouteDatagram& other) : WiseRouteDatagram_Base(other) {}
    WiseRouteDatagram& operator=(const WiseRouteDatagram& other) {WiseRouteDatagram_Base::operator=(other); return *this;}

    virtual WiseRouteDatagram *dup() const {return new WiseRouteDatagram(*this);}

    virtual Address getSourceAddress() const { return Address(getSrcAddr()); }
    virtual void setSourceAddress(const Address & address) { setSrcAddr(address.toModuleId()); }
    virtual Address getDestinationAddress() const { return Address(getDestAddr()); }
    virtual void setDestinationAddress(const Address & address) { setDestAddr(address.toModuleId()); }
    virtual int getTransportProtocol() const { return WiseRouteDatagram_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) { WiseRouteDatagram_Base::setTransportProtocol(protocol); };
};

}


#endif
