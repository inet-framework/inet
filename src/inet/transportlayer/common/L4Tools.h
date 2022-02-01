//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_L4TOOLS_H
#define __INET_L4TOOLS_H

#include "inet/common/ProtocolTools.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

namespace inet {

INET_API const Protocol *findTransportProtocol(Packet *packet);
INET_API const Protocol& getProtocolId(Packet *packet);

INET_API const Ptr<const TransportHeaderBase> findTransportProtocolHeader(Packet *packet);
INET_API const Ptr<const TransportHeaderBase> getTransportProtocolHeader(Packet *packet);

INET_API bool isTransportProtocol(const Protocol& protocol);

INET_API const Ptr<const TransportHeaderBase> peekTransportProtocolHeader(Packet *packet, const Protocol& protocol, int flags = 0);

INET_API void insertTransportProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<TransportHeaderBase>& header);

template<typename T>
const Ptr<T> removeTransportProtocolHeader(Packet *packet)
{
    packet->removeTagIfPresent<TransportProtocolInd>();
    return removeProtocolHeader<T>(packet);
}

INET_API const Ptr<TransportHeaderBase> removeTransportProtocolHeader(Packet *packet, const Protocol& protocol);

} // namespace inet

#endif

