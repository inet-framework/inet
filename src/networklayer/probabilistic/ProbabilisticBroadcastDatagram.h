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

#ifndef __INET_PROBABILISTICBROADCASTDATAGRAM_H
#define __INET_PROBABILISTICBROADCASTDATAGRAM_H

#include "INetworkDatagram.h"
#include "ProbabilisticBroadcastDatagram_m.h"

namespace inet {
/**
 * Represents an ProbabilisticBroadcast datagram. More info in the ProbabilisticBroadcastDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API ProbabilisticBroadcastDatagram : public ProbabilisticBroadcastDatagram_Base, public INetworkDatagram
{
  public:
    ProbabilisticBroadcastDatagram(const char *name = NULL, int kind = 0) : ProbabilisticBroadcastDatagram_Base(name, kind) {}
    ProbabilisticBroadcastDatagram(const ProbabilisticBroadcastDatagram& other) : ProbabilisticBroadcastDatagram_Base(other) {}
    ProbabilisticBroadcastDatagram& operator=(const ProbabilisticBroadcastDatagram& other) { ProbabilisticBroadcastDatagram_Base::operator=(other); return *this; }

    virtual ProbabilisticBroadcastDatagram *dup() const { return new ProbabilisticBroadcastDatagram(*this); }

    virtual Address getSourceAddress() const { return Address(getSrcAddr()); }
    virtual void setSourceAddress(const Address& address) { setSrcAddr(address.toModuleId()); }
    virtual Address getDestinationAddress() const { return Address(getDestAddr()); }
    virtual void setDestinationAddress(const Address& address) { setDestAddr(address.toModuleId()); }
    virtual int getTransportProtocol() const { return ProbabilisticBroadcastDatagram_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) { ProbabilisticBroadcastDatagram_Base::setTransportProtocol(protocol); };
};
} // namespace inet

#endif // ifndef __INET_PROBABILISTICBROADCASTDATAGRAM_H

