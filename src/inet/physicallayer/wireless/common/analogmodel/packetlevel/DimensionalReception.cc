//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"

namespace inet {

namespace physicallayer {

DimensionalReception::DimensionalReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    FlatReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, centerFrequency, bandwidth),
    power(power)
{
}

W DimensionalReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    Point<simsec> startPoint{ simsec(startTime) };
    Point<simsec> endPoint{ simsec(endTime) };
    W minPower = integrate<WpHz, Domain<simsec, Hz>, 0b10, W, Domain<simsec>>(power)->getMin(Interval<simsec>(startPoint, endPoint, 0b1, 0b1, 0b0));
    EV_DEBUG << "Computing minimum reception power" << EV_FIELD(startPoint) << EV_FIELD(endPoint) << EV_FIELD(minPower) << endl;
    return minPower;
}

} // namespace physicallayer

} // namespace inet

