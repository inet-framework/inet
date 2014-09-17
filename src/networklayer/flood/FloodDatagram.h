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

#ifndef __INET_FLOODDATAGRAM_H
#define __INET_FLOODDATAGRAM_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/networklayer/flood/FloodDatagram_m.h"

namespace inet {

/**
 * Represents an flood datagram. More info in the FloodDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API FloodDatagram : public FloodDatagram_Base, public INetworkDatagram
{
  public:
    FloodDatagram(const char *name = NULL, int kind = 0) : FloodDatagram_Base(name, kind) {}
    FloodDatagram(const FloodDatagram& other) : FloodDatagram_Base(other) {}
    FloodDatagram& operator=(const FloodDatagram& other) { FloodDatagram_Base::operator=(other); return *this; }

    virtual FloodDatagram *dup() const { return new FloodDatagram(*this); }

    virtual L3Address getSourceAddress() const { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) { setSrcAddr(address.toModuleId()); }
    virtual L3Address getDestinationAddress() const { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) { setDestAddr(address.toModuleId()); }
    virtual int getTransportProtocol() const { return FloodDatagram_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) { FloodDatagram_Base::setTransportProtocol(protocol); };
};

} // namespace inet

#endif // ifndef __INET_FLOODDATAGRAM_H

