//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/AnalogModelBase.h"

#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

namespace physicallayer {

double AnalogModelBase::computeAntennaGain(const IAntennaGain *antennaGain, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation) const
{
    if (antennaGain->getMinGain() == antennaGain->getMaxGain())
        return antennaGain->getMinGain();
    else {
        auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, endPosition - startPosition);
        auto antennaLocalDirection = Quaternion(startOrientation).inverse() * direction;
        return antennaGain->computeGain(antennaLocalDirection);
    }
}

} // namespace physicallayer

} // namespace inet

