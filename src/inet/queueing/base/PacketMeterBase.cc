//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketMeterBase.h"

namespace inet {
namespace queueing {

cGate *PacketMeterBase::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void PacketMeterBase::processPacket(Packet *packet)
{
    EV_INFO << "Metering packet" << EV_FIELD(packet) << EV_ENDL;
    meterPacket(packet);
}

} // namespace queueing
} // namespace inet

