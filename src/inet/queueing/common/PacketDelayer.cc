//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketDelayer.h"

namespace inet {
namespace queueing {

Define_Module(PacketDelayer);

void PacketDelayer::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        delayParameter = &par("delay");
        bitrateParameter = &par("bitrate");
    }
}

clocktime_t PacketDelayer::computeDelay(Packet *packet) const
{
    return delayParameter->doubleValue() + s(packet->getDataLength() / bps(bitrateParameter->doubleValue())).get();
}

} // namespace queueing
} // namespace inet

