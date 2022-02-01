//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETEVENTTAG_H
#define __INET_PACKETEVENTTAG_H

#include "inet/common/PacketEventTag_m.h"
#include "inet/common/packet/Packet.h"

namespace inet {

INET_API void insertPacketEvent(const cModule *module, Packet *packet, int kind, simtime_t duration, PacketEvent *packetEvent = new PacketEvent());

} // namespace inet

#endif

