//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

namespace inet {

namespace physicallayer {

UnitDiskTransmission::UnitDiskTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    TransmissionBase(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation)
{
}

} // namespace physicallayer

} // namespace inet

