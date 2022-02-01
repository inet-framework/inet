//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/function/PacketFilterFunction.h"

namespace inet {
namespace queueing {

static bool filterAnyPacket(const Packet *packet)
{
    return true;
}

Register_Packet_Filter_Function(AnyPacketFilter, filterAnyPacket);

} // namespace queueing
} // namespace inet

