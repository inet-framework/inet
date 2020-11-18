//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

ScalarSnir::ScalarSnir(const IReception *reception, const INoise *noise) :
    SnirBase(reception, noise),
    minSNIR(NaN),
    maxSNIR(NaN),
    meanSNIR(NaN)
{
}

std::ostream& ScalarSnir::printToStream(std::ostream& stream, int level) const
{
    stream << "ScalarSnir";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", minSNIR = " << minSNIR;
    return stream;
}

double ScalarSnir::computeMin() const
{
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(reception->getAnalogModel());
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    std::cout << "computeMin(): " << scalarSignalAnalogModel->getPower() << " " << scalarNoise->computeMaxPower(reception->getStartTime(), reception->getEndTime()).get() << "\n";
    return unit(scalarSignalAnalogModel->getPower() / scalarNoise->computeMaxPower(reception->getStartTime(), reception->getEndTime())).get();
}

double ScalarSnir::computeMax() const
{
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(reception->getAnalogModel());
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    return unit(scalarSignalAnalogModel->getPower() / scalarNoise->computeMinPower(reception->getStartTime(), reception->getEndTime())).get();
}

double ScalarSnir::getMin() const
{
    if (std::isnan(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

double ScalarSnir::getMax() const
{
    if (std::isnan(maxSNIR))
        maxSNIR = computeMax();
    return maxSNIR;
}

double ScalarSnir::getMean() const
{
    if (std::isnan(meanSNIR))
        meanSNIR = computeMean(reception->getStartTime(), reception->getEndTime());
    return meanSNIR;
}

double ScalarSnir::computeMean(simtime_t startTime, simtime_t endTime) const
{
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(reception->getAnalogModel());
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    const auto& signalPowerFunction = makeShared<math::ConstantFunction<W, math::Domain<simtime_t>>>(scalarSignalAnalogModel->getPower());
    const auto& snirFunction = signalPowerFunction->divide(scalarNoise->getPower());
    math::Point<simtime_t> startPoint(startTime);
    math::Point<simtime_t> endPoint(endTime);
    math::Interval<simtime_t> interval(startPoint, endPoint, 0b1, 0b0, 0b0);
    return snirFunction->getMean(interval);
}

} // namespace physicallayer

} // namespace inet

