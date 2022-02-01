//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_L3TOOLS_H
#define __INET_L3TOOLS_H

#include "inet/common/ProtocolTools.h"
#include "inet/networklayer/contract/NetworkHeaderBase_m.h"

namespace inet {

INET_API const Protocol *findNetworkProtocol(Packet *packet);
INET_API const Protocol& getNetworkProtocol(Packet *packet);

INET_API const Ptr<const NetworkHeaderBase> findNetworkProtocolHeader(Packet *packet);
INET_API const Ptr<const NetworkHeaderBase> getNetworkProtocolHeader(Packet *packet);

INET_API const Ptr<const NetworkHeaderBase> peekNetworkProtocolHeader(const Packet *packet, const Protocol& protocol);

INET_API void insertNetworkProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<NetworkHeaderBase>& header);

template<typename T>
const Ptr<T> removeNetworkProtocolHeader(Packet *packet)
{
    packet->removeTagIfPresent<NetworkProtocolInd>();
    return removeProtocolHeader<T>(packet);
}

INET_API const Ptr<NetworkHeaderBase> removeNetworkProtocolHeader(Packet *packet, const Protocol& protocol);

} // namespace inet

#endif

