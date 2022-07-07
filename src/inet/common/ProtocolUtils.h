//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLUTILS_H
#define __INET_PROTOCOLUTILS_H

#include "inet/common/packet/Packet.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

extern void dispatchToNextEncapsulationProtocol(Packet *packet);
extern void appendDispatchToNetworkInterface(Packet *packet, const NetworkInterface *networkInterface);
extern void appendDispatchToEncapsulationProtocol(Packet *packet, const Protocol *protocol);
extern void prependDispatchToEncapsulationProtocol(Packet *packet, const Protocol *protocol);

} // namespace inet

#endif

