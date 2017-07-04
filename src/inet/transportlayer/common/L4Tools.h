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

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

namespace inet {
    const Ptr<const TransportHeaderBase> peekTransportHeader(Packet *packet);
    const Ptr<const TransportHeaderBase> peekTransportHeader(const Protocol *protocol, Packet *packet);
    const Ptr<TransportHeaderBase> removeTransportHeader(Packet *packet);
    const Ptr<TransportHeaderBase> removeTransportHeader(const Protocol *protocol, Packet *packet);
};

#endif    // __INET_L4TOOLS_H
