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


#ifndef _INET_GENERICDATAGRAM_H_
#define _INET_GENERICDATAGRAM_H_

#include "INETDefs.h"
#include "INetworkDatagram.h"
#include "GenericDatagram_m.h"
#include "IPProtocolId_m.h"

/**
 * Represents a generic datagram. More info in the GenericDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API GenericDatagram : public GenericDatagram_Base, public INetworkDatagram
{
  public:
    GenericDatagram(const char *name = NULL, int kind = 0) : GenericDatagram_Base(name, kind) {}
    GenericDatagram(const GenericDatagram& other) : GenericDatagram_Base(other.getName()) {operator=(other);}
    GenericDatagram& operator=(const GenericDatagram& other) {GenericDatagram_Base::operator=(other); return *this;}
    virtual GenericDatagram *dup() const {return new GenericDatagram(*this);}

    virtual Address getSourceAddress() const {return GenericDatagram_Base::_getSrcAddr();}
    virtual void setSourceAddress(const Address& addr)  {GenericDatagram_Base::setSourceAddress(addr);}
    virtual Address getDestinationAddress() const  {return GenericDatagram_Base::_getDestAddr();}
    virtual void setDestinationAddress(const Address& addr)  {GenericDatagram_Base::setDestinationAddress(addr);}
    virtual int getTransportProtocol() const {return GenericDatagram_Base::getTransportProtocol();}
    virtual void setTransportProtocol(int protocol) {GenericDatagram_Base::setTransportProtocol(protocol);}
};

#endif
