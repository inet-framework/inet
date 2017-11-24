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

#ifndef __INET_FloodHeader_H
#define __INET_FloodHeader_H

#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/flood/FloodHeader_m.h"

namespace inet {

/**
 * Represents an flood datagram. More info in the FloodHeader.msg file
 * (and the documentation generated from it).
 */
class INET_API FloodHeader : public FloodHeader_Base
{
  public:
    FloodHeader() : FloodHeader_Base() {}
    FloodHeader(const FloodHeader& other) : FloodHeader_Base(other) {}
    FloodHeader& operator=(const FloodHeader& other) { FloodHeader_Base::operator=(other); return *this; }

    virtual FloodHeader *dup() const override { return new FloodHeader(*this); }

    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address); }
    virtual ConstProtocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(ConstProtocol *protocol) override { setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(protocol)); }
};

} // namespace inet

#endif // ifndef __INET_FloodHeader_H

