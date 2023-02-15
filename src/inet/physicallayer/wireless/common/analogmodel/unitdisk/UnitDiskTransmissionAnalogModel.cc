//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskTransmissionAnalogModel.h"

namespace inet {
namespace physicallayer {

UnitDiskTransmissionAnalogModel::UnitDiskTransmissionAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, m communicationRange, m interferenceRange, m detectionRange) :
    SignalAnalogModel(preambleDuration, headerDuration, dataDuration),
    communicationRange(communicationRange),
    interferenceRange(interferenceRange),
    detectionRange(detectionRange)
{
}

} // namespace physicallayer
} // namespace inet

