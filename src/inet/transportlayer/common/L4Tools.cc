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
#include "inet/transportlayer/common/L4Tools.h"

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif
#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader.h"
#endif
#ifdef WITH_SCTP
//TODO
#endif

namespace inet {

Ptr<TransportHeaderBase> peekTransportHeader(Packet *packet)
{
    auto protocol = packet->getMandatoryTag<PacketProtocolTag>()->getProtocol();
    return peekTransportHeader(protocol, packet);
}

Ptr<TransportHeaderBase> peekTransportHeader(const Protocol *protocol, Packet *packet)
{
#ifdef WITH_TCP_COMMON
    if (protocol == &Protocol::tcp)
        return packet->peekHeader<tcp::TcpHeader>();
#endif
#ifdef WITH_UDP
    if (protocol == &Protocol::udp)
        return packet->peekHeader<UdpHeader>();
#endif

    //TODO add other L4 protocols

    throw cRuntimeError("Unacceptable protocol %s", protocol->getName());
}

Ptr<TransportHeaderBase> removeTransportHeader(Packet *packet)
{
    auto protocol = packet->getMandatoryTag<PacketProtocolTag>()->getProtocol();
    return removeTransportHeader(protocol, packet);
}

Ptr<TransportHeaderBase> removeTransportHeader(const Protocol *protocol, Packet *packet)
{
#ifdef WITH_TCP_COMMON
    if (protocol == &Protocol::tcp)
        return packet->removeHeader<tcp::TcpHeader>();
#endif
#ifdef WITH_UDP
    if (protocol == &Protocol::udp)
        return packet->removeHeader<UdpHeader>();
#endif

    //TODO add other L4 protocols

    throw cRuntimeError("Unacceptable protocol %s", protocol->getName());
}

} // namespace inet

