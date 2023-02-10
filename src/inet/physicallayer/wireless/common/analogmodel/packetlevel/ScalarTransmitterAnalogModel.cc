//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmitterAnalogModel.h"

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

INewTransmissionAnalogModel *ScalarTransmitterAnalogModel::createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const
{
    return new ScalarTransmissionAnalogModel(centerFrequency, bandwidth, power);
}

} // namespace physicallayer
} // namespace inet

