//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"

namespace inet {
namespace physicallayer {

DimensionalSignalAnalogModel::DimensionalSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    NarrowbandSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalSignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalSignalAnalogModel";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(powerMax, power->getMax())
               << EV_FIELD(powerMin, power->getMin());
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(power);
    return NarrowbandSignalAnalogModel::printToStream(stream, level);
}

W DimensionalSignalAnalogModel::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    Point<simsec> startPoint{ simsec(startTime) };
    Point<simsec> endPoint{ simsec(endTime) };
    W minPower = integrate<WpHz, Domain<simsec, Hz>, 0b10, W, Domain<simsec>>(power)->getMin(Interval<simsec>(startPoint, endPoint, 0b1, 0b1, 0b0));
    EV_DEBUG << "Computing minimum reception power" << EV_FIELD(startPoint) << EV_FIELD(endPoint) << EV_FIELD(minPower) << endl;
    return minPower;
}

DimensionalTransmissionSignalAnalogModel::DimensionalTransmissionSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    DimensionalSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, bandwidth, power)
{
}

DimensionalReceptionSignalAnalogModel::DimensionalReceptionSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    DimensionalSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

