//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskTransmitterAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskTransmitterAnalogModel);

void UnitDiskTransmitterAnalogModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        communicationRange = m(par("communicationRange"));
        interferenceRange = m(par("interferenceRange"));
        detectionRange = m(par("detectionRange"));
    }
}

ITransmissionAnalogModel* UnitDiskTransmitterAnalogModel::createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const
{
    return new UnitDiskTransmissionAnalogModel(-1, -1, duration, communicationRange, interferenceRange, detectionRange);
}

} // namespace physicallayer
} // namespace inet

