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

const Protocol *findTransportProtocol(Packet *packet)
{
    auto transportProtocolInd = packet->getTag<TransportProtocolInd>();
    return transportProtocolInd == nullptr ? nullptr : transportProtocolInd->getProtocol();
}

const Protocol& getTransportProtocol(Packet *packet)
{
    auto protocol = findTransportProtocol(packet);
    if (protocol == nullptr)
        throw cRuntimeError("Transport protocol not found");
    else
        return *protocol;
}

const Ptr<const TransportHeaderBase> findTransportProtocolHeader(Packet *packet)
{
    auto transportProtocolInd = packet->getTag<TransportProtocolInd>();
    return transportProtocolInd == nullptr ? nullptr : std::dynamic_pointer_cast<const TransportHeaderBase>(transportProtocolInd->getTransportProtocolHeader());
}

const Ptr<const TransportHeaderBase> getTransportProtocolHeader(Packet *packet)
{
    const auto& header = findTransportProtocolHeader(packet);
    if (header == nullptr)
        throw cRuntimeError("Transport protocol header not found");
    else
        return header;
}

const Ptr<const TransportHeaderBase> peekTransportProtocolHeader(Packet *packet, const Protocol& protocol)
{
#ifdef WITH_TCP_COMMON
    if (protocol == Protocol::tcp)
        return packet->peekHeader<tcp::TcpHeader>();
#endif
#ifdef WITH_UDP
    if (protocol == Protocol::udp)
        return packet->peekHeader<UdpHeader>();
#endif
    // TODO: add other L4 protocols
    throw cRuntimeError("Unknown protocol: %s", protocol.getName());
}

void insertTransportProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<TransportHeaderBase>& header)
{
    auto transportProtocolInd = packet->ensureTag<TransportProtocolInd>();
    transportProtocolInd->setProtocol(&Protocol::ipv4);
    transportProtocolInd->setTransportProtocolHeader(header);
    insertProtocolHeader(packet, protocol, header);
}

const Ptr<TransportHeaderBase> removeTransportProtocolHeader(Packet *packet, const Protocol& protocol)
{
#ifdef WITH_TCP_COMMON
    if (protocol == Protocol::tcp)
        return removeTransportProtocolHeader<tcp::TcpHeader>(packet);
#endif
#ifdef WITH_UDP
    if (protocol == Protocol::udp)
        return removeTransportProtocolHeader<UdpHeader>(packet);
#endif
    // TODO: add other L4 protocols
    throw cRuntimeError("Unknown protocol: %s", protocol.getName());
}

} // namespace inet

