//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketMarkerBase.h"

namespace inet {
namespace queueing {

void PacketMarkerBase::processPacket(Packet *packet)
{
    EV_INFO << "Marking packet" << EV_FIELD(packet) << EV_ENDL;
    markPacket(packet);
}

} // namespace queueing
} // namespace inet

