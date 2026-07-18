//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/backoff/ExponentialBackoff.h"

#include <algorithm>

#include "inet/protocolelement/common/TransmissionAttemptTag_m.h"

namespace inet {

Define_Module(ExponentialBackoff);

void ExponentialBackoff::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        slotTime = par("slotTime");
        minBackoffExponent = par("minBackoffExponent");
        maxBackoffExponent = par("maxBackoffExponent");
    }
}

clocktime_t ExponentialBackoff::computeDelay(Packet *packet) const
{
    int attempt = 0;
    if (auto tag = packet->findTag<TransmissionAttemptReq>())
        attempt = tag->getAttempt();
    int exponent = std::min(minBackoffExponent + attempt, maxBackoffExponent);
    int contentionWindow = (1 << exponent) - 1;
    int slots = intuniform(0, contentionWindow);
    return slotTime.dbl() * slots;
}

} // namespace inet

