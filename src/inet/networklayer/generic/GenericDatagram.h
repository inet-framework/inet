//
// Copyright (C) 2012 Opensim Ltd.
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

#ifndef __INET_GENERICDATAGRAM_H
#define __INET_GENERICDATAGRAM_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/networklayer/generic/GenericDatagram_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {

/**
 * Represents a generic datagram. More info in the GenericDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API GenericDatagramHeader : public GenericDatagramHeader_Base, public INetworkDatagram
{
  public:
    GenericDatagramHeader() : GenericDatagramHeader_Base() {}
    GenericDatagramHeader(const GenericDatagramHeader& other) : GenericDatagramHeader_Base() { operator=(other); }
    GenericDatagramHeader& operator=(const GenericDatagramHeader& other) { GenericDatagramHeader_Base::operator=(other); return *this; }
    virtual GenericDatagramHeader *dup() const override { return new GenericDatagramHeader(*this); }

    virtual L3Address getSourceAddress() const override { return GenericDatagramHeader_Base::_getSrcAddr(); }
    virtual void setSourceAddress(const L3Address& addr) override { GenericDatagramHeader_Base::setSourceAddress(addr); }
    virtual L3Address getDestinationAddress() const override { return GenericDatagramHeader_Base::_getDestAddr(); }
    virtual void setDestinationAddress(const L3Address& addr) override { GenericDatagramHeader_Base::setDestinationAddress(addr); }
    virtual int getTransportProtocol() const override { return GenericDatagramHeader_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) override { GenericDatagramHeader_Base::setTransportProtocol(protocol); }
};

} // namespace inet

#endif // ifndef __INET_GENERICDATAGRAM_H

