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
        Coord direction = endPosition - startPosition;
        Quaternion r1 = Quaternion::rotationFromTo(Coord::X_AXIS, direction); // rotation from X axis to the reception direction in the global coordinate system
        Quaternion r2 = Quaternion(startOrientation).inverse() * r1; // rotation from X axis to the reception direction in the antenna's local coordinate system
        return antennaGain->computeGain(r2);
    }
}

} // namespace physicallayer

} // namespace inet

