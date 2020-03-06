//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarNoise.h"

namespace inet {

namespace physicallayer {

ScalarNoise::ScalarNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, Ptr<const math::IFunction<W, math::Domain<simtime_t>>> powerFunction) :
    NarrowbandNoiseBase(startTime, endTime, centerFrequency, bandwidth),
    powerFunction(powerFunction)
{
}

std::ostream& ScalarNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerFunction);
    return NarrowbandNoiseBase::printToStream(stream, level);
}

W ScalarNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    math::Point<simtime_t> startPoint(startTime);
    math::Point<simtime_t> endPoint(endTime);
    math::Interval<simtime_t> interval(startPoint, endPoint, 0b1, 0b1, 0b0);
    return powerFunction->getMin(interval);
}

W ScalarNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    math::Point<simtime_t> startPoint(startTime);
    math::Point<simtime_t> endPoint(endTime);
    math::Interval<simtime_t> interval(startPoint, endPoint, 0b1, 0b1, 0b0);
    return powerFunction->getMax(interval);
}

} // namespace physicallayer

} // namespace inet

