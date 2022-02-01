//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/common/L4Tools.h"

#ifdef INET_WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif

#ifdef INET_WITH_SCTP
// TODO
#endif

namespace inet {

const Protocol *findTransportProtocol(Packet *packet)
{
    const auto& transportProtocolInd = packet->findTag<TransportProtocolInd>();
    return transportProtocolInd == nullptr ? nullptr : transportProtocolInd->getProtocol();
}

const Protocol& getProtocolId(Packet *packet)
{
    auto protocol = findTransportProtocol(packet);
    if (protocol == nullptr)
        throw cRuntimeError("Transport protocol not found");
    else
        return *protocol;
}

const Ptr<const TransportHeaderBase> findTransportProtocolHeader(Packet *packet)
{
    const auto& transportProtocolInd = packet->findTag<TransportProtocolInd>();
    return transportProtocolInd == nullptr ? nullptr : dynamicPtrCast<const TransportHeaderBase>(transportProtocolInd->getTransportProtocolHeader());
}

const Ptr<const TransportHeaderBase> getTransportProtocolHeader(Packet *packet)
{
    const auto& header = findTransportProtocolHeader(packet);
    if (header == nullptr)
        throw cRuntimeError("Transport protocol header not found");
    else
        return header;
}

bool isTransportProtocol(const Protocol& protocol)
{
    // TODO add other L4 protocols
    return protocol == Protocol::tcp ||
           protocol == Protocol::udp;
}

const Ptr<const TransportHeaderBase> peekTransportProtocolHeader(Packet *packet, const Protocol& protocol, int flags)
{
#ifdef INET_WITH_TCP_COMMON
    if (protocol == Protocol::tcp)
        return packet->peekAtFront<tcp::TcpHeader>(b(-1), flags);
#endif
#ifdef INET_WITH_UDP
    if (protocol == Protocol::udp)
        return packet->peekAtFront<UdpHeader>(b(-1), flags);
#endif
    // TODO add other L4 protocols
    if (flags & Chunk::PF_ALLOW_NULLPTR)
        return nullptr;
    throw cRuntimeError("Unknown protocol: %s", protocol.getName());
}

void insertTransportProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<TransportHeaderBase>& header)
{
    auto transportProtocolInd = packet->addTagIfAbsent<TransportProtocolInd>();
    transportProtocolInd->setProtocol(&protocol);
    transportProtocolInd->setTransportProtocolHeader(header);
    insertProtocolHeader(packet, protocol, header);
}

const Ptr<TransportHeaderBase> removeTransportProtocolHeader(Packet *packet, const Protocol& protocol)
{
#ifdef INET_WITH_TCP_COMMON
    if (protocol == Protocol::tcp)
        return removeTransportProtocolHeader<tcp::TcpHeader>(packet);
#endif
#ifdef INET_WITH_UDP
    if (protocol == Protocol::udp)
        return removeTransportProtocolHeader<UdpHeader>(packet);
#endif
    // TODO add other L4 protocols
    throw cRuntimeError("Unknown protocol: %s", protocol.getName());
}

} // namespace inet

