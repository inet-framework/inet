//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLOWTAG_H
#define __INET_FLOWTAG_H

#include "inet/common/FlowTag_m.h"
#include "inet/common/packet/Packet.h"

namespace inet {

INET_API void startPacketFlow(cModule *module, Packet *packet, const char *name);

INET_API void endPacketFlow(cModule *module, Packet *packet, const char *name);

} // namespace inet

#endif

