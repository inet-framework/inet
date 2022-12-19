//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLUTILS_H
#define __INET_PROTOCOLUTILS_H

#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

INET_API void pushEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API const Protocol *popEncapsulationProtocolReq(Packet *packet);

INET_API bool hasEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API void ensureEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);

INET_API void pushEncapsulationProtocolInd(Packet *packet, const Protocol *protocol);

} // namespace inet

#endif

