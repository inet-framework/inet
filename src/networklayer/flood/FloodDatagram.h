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


#ifndef _FLOODDATAGRAM_H_
#define _FLOODDATAGRAM_H_

#include "INETDefs.h"
#include "INetworkDatagram.h"
#include "FloodDatagram_m.h"
// TODO: do we really need this?
#include "IPProtocolId_m.h"

/**
 * Represents an flood datagram. More info in the FloodDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API FloodDatagram : public FloodDatagram_Base, public INetworkDatagram
{
  public:
    FloodDatagram(const char *name = NULL, int kind = 0) : FloodDatagram_Base(name, kind) {}
    FloodDatagram(const FloodDatagram& other) : FloodDatagram_Base(other) {}
    FloodDatagram& operator=(const FloodDatagram& other) {FloodDatagram_Base::operator=(other); return *this;}

    virtual FloodDatagram *dup() const {return new FloodDatagram(*this);}

    virtual Address getSourceAddress() const { return Address(getSrcAddress()); }
    virtual void setSourceAddress(const Address & address) { setSrcAddress(address.toModuleId()); }
    virtual Address getDestinationAddress() const { return Address(getDestAddress()); }
    virtual void setDestinationAddress(const Address & address) { setDestAddress(address.toModuleId()); }
    // TODO: do we really need this?
    virtual int getTransportProtocol() const { return IP_PROT_NONE; }
    virtual void setTransportProtocol(int protocol) { ASSERT(false); };
};

#endif
