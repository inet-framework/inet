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

INET_API void prependEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API void appendEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API const Protocol *peekEncapsulationProtocolReq(Packet *packet);
INET_API const Protocol *popEncapsulationProtocolReq(Packet *packet);

INET_API bool hasEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API void removeEncapsulationProtocolReq(Packet *packet, const Protocol *protocol);
INET_API void ensureEncapsulationProtocolReq(Packet *packet, const Protocol *protocol, bool present = true, bool prepend = true);

INET_API void prependEncapsulationProtocolInd(Packet *packet, const Protocol *protocol);
INET_API void appendEncapsulationProtocolInd(Packet *packet, const Protocol *protocol);

INET_API void setDispatchProtocol(Packet *packet, const Protocol *defaultProtocol = nullptr);
INET_API void removeDispatchProtocol(Packet *packet, const Protocol *expectedProtocol);

} // namespace inet

#endif

