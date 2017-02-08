//
// Copyright (C) 2017 OpenSim Ltd.
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

#include "inet/common/INETDefs.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/contract/INetfilter.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Header.h"
#endif
#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Header.h"
#endif
#ifdef WITH_GENERIC
#include "inet/networklayer/generic/GenericDatagram.h"
#endif

namespace inet {

std::shared_ptr<INetworkHeader> NetfilterBase::HookBase::peekNetworkHeader(Packet *packet)
{
    auto protocol = packet->getMandatoryTag<PacketProtocolTag>()->getProtocol();

#ifdef WITH_IPv4
    if (protocol == &Protocol::ipv4)
        return packet->peekHeader<IPv4Header>();
#endif
#ifdef WITH_IPv6
    if (protocol == &Protocol::ipv6)
        return packet->peekHeader<IPv6Header>();
#endif
#ifdef WITH_GENERIC
    if (protocol == &Protocol::gnp)
        return packet->peekHeader<GenericDatagramHeader>();
#endif
    throw cRuntimeError("Unacceptable protocol %s", protocol->getName());
}

} // namespace inet


