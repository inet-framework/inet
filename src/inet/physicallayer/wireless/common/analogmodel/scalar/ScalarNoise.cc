//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarNoise.h"

#include "inet/common/math/Functions.h"

namespace inet {

namespace physicallayer {

ScalarNoise::ScalarNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, Ptr<const math::IFunction<W, math::Domain<simtime_t>>> powerFunction, const std::vector<PowerComponent>& powerComponents) :
    NarrowbandNoiseBase(startTime, endTime, centerFrequency, bandwidth),
    powerFunction(powerFunction),
    powerComponents(powerComponents.empty() ? std::vector<PowerComponent>{{centerFrequency, bandwidth, powerFunction}} : powerComponents)
{
}

std::ostream& ScalarNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerFunction);
    return NarrowbandNoiseBase::printToStream(stream, level);
}

Ptr<const math::IFunction<W, math::Domain<simtime_t>>> ScalarNoise::getPower(Hz centerFrequency, Hz bandwidth) const
{
    if (centerFrequency == this->centerFrequency && bandwidth == this->bandwidth)
        return powerFunction;
    Ptr<const math::IFunction<W, math::Domain<simtime_t>>> result = makeShared<math::ConstantFunction<W, math::Domain<simtime_t>>>(W(0));
    for (const auto& component : powerComponents) {
        auto lower = std::max(centerFrequency - bandwidth / 2, component.centerFrequency - component.bandwidth / 2);
        auto upper = std::min(centerFrequency + bandwidth / 2, component.centerFrequency + component.bandwidth / 2);
        if (upper > lower) {
            double scale = ((upper - lower) / component.bandwidth).get<unit>();
            auto scaleFunction = makeShared<math::ConstantFunction<double, math::Domain<simtime_t>>>(scale);
            result = result->add(component.powerFunction->multiply(scaleFunction));
        }
    }
    return result;
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

