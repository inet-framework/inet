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

#ifndef __INET_ProbabilisticBroadcastHeader_H
#define __INET_ProbabilisticBroadcastHeader_H

#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/contract/NetworkHeaderBase_m.h"
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"

namespace inet {

/**
 * Represents an ProbabilisticBroadcast datagram. More info in the ProbabilisticBroadcastHeader.msg file
 * (and the documentation generated from it).
 */
class INET_API ProbabilisticBroadcastHeader : public ProbabilisticBroadcastHeader_Base
{
  public:
    ProbabilisticBroadcastHeader() : ProbabilisticBroadcastHeader_Base() {}
    ProbabilisticBroadcastHeader(const ProbabilisticBroadcastHeader& other) : ProbabilisticBroadcastHeader_Base(other) {}
    ProbabilisticBroadcastHeader& operator=(const ProbabilisticBroadcastHeader& other) { ProbabilisticBroadcastHeader_Base::operator=(other); return *this; }

    virtual ProbabilisticBroadcastHeader *dup() const override { return new ProbabilisticBroadcastHeader(*this); }

    virtual L3Address getSourceAddress() const override { return getSrcAddr(); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address); }
    virtual L3Address getDestinationAddress() const override { return getDestAddr(); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address); }
    virtual ConstProtocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(ConstProtocol *protocol) override { setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(protocol)); }
};

} // namespace inet

#endif // ifndef __INET_ProbabilisticBroadcastHeader_H

