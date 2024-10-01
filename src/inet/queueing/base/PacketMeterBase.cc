//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketMeterBase.h"

namespace inet {
namespace queueing {

void PacketMeterBase::processPacket(Packet *packet)
{
    EV_INFO << "Metering packet" << EV_FIELD(packet) << EV_ENDL;
    meterPacket(packet);
}

} // namespace queueing
} // namespace inet

