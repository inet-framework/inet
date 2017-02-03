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

#include "inet/networklayer/contract/INetworkHeader.h"
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"

namespace inet {

/**
 * Represents an ProbabilisticBroadcast datagram. More info in the ProbabilisticBroadcastHeader.msg file
 * (and the documentation generated from it).
 */
class INET_API ProbabilisticBroadcastHeader : public ProbabilisticBroadcastHeader_Base, public INetworkHeader
{
  public:
    ProbabilisticBroadcastHeader() : ProbabilisticBroadcastHeader_Base() {}
    ProbabilisticBroadcastHeader(const ProbabilisticBroadcastHeader& other) : ProbabilisticBroadcastHeader_Base(other) {}
    ProbabilisticBroadcastHeader& operator=(const ProbabilisticBroadcastHeader& other) { ProbabilisticBroadcastHeader_Base::operator=(other); return *this; }

    virtual ProbabilisticBroadcastHeader *dup() const override { return new ProbabilisticBroadcastHeader(*this); }

    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddr()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddr(address.toModuleId()); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddr()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddr(address.toModuleId()); }
    virtual int getTransportProtocol() const override { return ProbabilisticBroadcastHeader_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) override { ProbabilisticBroadcastHeader_Base::setTransportProtocol(protocol); };
};

} // namespace inet

#endif // ifndef __INET_ProbabilisticBroadcastHeader_H

