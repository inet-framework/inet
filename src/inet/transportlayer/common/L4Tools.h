//
// Copyright (C) 2017 OpenSim Ltd.
// @author: Zoltan Bojthe
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

#ifndef __INET_L4TOOLS_H
#define __INET_L4TOOLS_H

#include "inet/common/ProtocolTools.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

namespace inet {

const Protocol *findTransportProtocol(Packet *packet);
const Protocol& getProtocolId(Packet *packet);

const Ptr<const TransportHeaderBase> findTransportProtocolHeader(Packet *packet);
const Ptr<const TransportHeaderBase> getTransportProtocolHeader(Packet *packet);

bool isTransportProtocol(const Protocol& protocol);

const Ptr<const TransportHeaderBase> peekTransportProtocolHeader(Packet *packet, const Protocol& protocol, int flags = 0);

void insertTransportProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<TransportHeaderBase>& header);

template <typename T>
const Ptr<T> removeTransportProtocolHeader(Packet *packet)
{
    delete packet->removeTagIfPresent<TransportProtocolInd>();
    return removeProtocolHeader<T>(packet);
}

const Ptr<TransportHeaderBase> removeTransportProtocolHeader(Packet *packet, const Protocol& protocol);

}

#endif // __INET_L4TOOLS_H
