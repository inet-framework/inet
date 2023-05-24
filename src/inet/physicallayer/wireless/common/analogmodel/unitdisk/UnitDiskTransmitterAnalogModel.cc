//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskTransmitterAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskTransmissionAnalogModel.h"

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

ITransmissionAnalogModel* UnitDiskTransmitterAnalogModel::createAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power) const
{
    return new UnitDiskTransmissionAnalogModel(preambleDuration, headerDuration, dataDuration, communicationRange, interferenceRange, detectionRange);
}

} // namespace physicallayer
} // namespace inet

