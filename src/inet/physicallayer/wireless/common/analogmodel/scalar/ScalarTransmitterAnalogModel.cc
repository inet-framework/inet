//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarTransmitterAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(ScalarTransmitterAnalogModel);

void ScalarTransmitterAnalogModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        defaultCenterFrequency = Hz(par("centerFrequency"));
        defaultBandwidth = Hz(par("bandwidth"));
        defaultPower = W(par("power"));
    }
}

ITransmissionAnalogModel *ScalarTransmitterAnalogModel::createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const
{
    auto transmissionCenterFrequency = computeCenterFrequency(centerFrequency);
    auto transmissionBandwidth = computeBandwidth(bandwidth);
    auto transmissionPower = computePower(power);
    return new ScalarTransmissionSignalAnalogModel(-1, -1, duration, transmissionCenterFrequency, transmissionBandwidth, transmissionPower);
}

} // namespace physicallayer
} // namespace inet

