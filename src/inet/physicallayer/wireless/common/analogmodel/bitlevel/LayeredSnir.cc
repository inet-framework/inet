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

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredSnir.h"

namespace inet {

namespace physicallayer {

LayeredSnir::LayeredSnir(const LayeredReception *reception, const DimensionalNoise *noise) :
    SnirBase(reception, noise),
    minSNIR(NaN),
    maxSNIR(NaN),
    meanSNIR(NaN)
{
}

std::ostream& LayeredSnir::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredSnir";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", minSNIR = " << minSNIR;
    return SnirBase::printToStream(stream, level);
}

double LayeredSnir::computeMin() const
{
    // TODO: factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const IDimensionalSignal *dimensionalSignal = check_and_cast<const IDimensionalSignal *>(reception->getAnalogModel());
    const INarrowbandSignal *narrowbandSignal = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
    EV_TRACE << "Reception power begin " << endl;
//    EV_TRACE << *dimensionalSignal->getPower() << endl;
    EV_TRACE << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalSignal->getPower();
    auto snir = receptionPower->divide(noisePower);
    simsec startTime = simsec(reception->getStartTime());
    simsec endTime = simsec(reception->getEndTime());
    Hz centerFrequency = narrowbandSignal->getCenterFrequency();
    Hz bandwidth = narrowbandSignal->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
//    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double minSNIR = snir->getMin(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing minimum SNIR: start = " << startPoint << ", end = " << endPoint << " -> minimum SNIR = " << minSNIR << endl;
    return minSNIR;
}

double LayeredSnir::computeMax() const
{
    // TODO: factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const IDimensionalSignal *dimensionalSignal = check_and_cast<const IDimensionalSignal *>(reception->getAnalogModel());
    const INarrowbandSignal *narrowbandSignal = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
    EV_DEBUG << "Reception power begin " << endl;
    EV_DEBUG <<* dimensionalSignal->getPower() << endl;
    EV_DEBUG << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalSignal->getPower();
    auto snir = receptionPower->divide(noisePower);
    auto startTime = simsec(reception->getStartTime());
    auto endTime = simsec(reception->getEndTime());
    Hz centerFrequency = narrowbandSignal->getCenterFrequency();
    Hz bandwidth = narrowbandSignal->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
//    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double maxSNIR = snir->getMax(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing maximum SNIR: start = " << startPoint << ", end = " << endPoint << " -> maximum SNIR = " << maxSNIR << endl;
    return maxSNIR;
}

double LayeredSnir::computeMean() const
{
    // TODO: factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const IDimensionalSignal *dimensionalSignal = check_and_cast<const IDimensionalSignal *>(reception->getAnalogModel());
    const INarrowbandSignal *narrowbandSignal = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
    EV_TRACE << "Reception power begin " << endl;
//    EV_TRACE << *dimensionalSignal->getPower() << endl;
    EV_TRACE << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalSignal->getPower();
    auto snir = receptionPower->divide(noisePower);
    auto startTime = simsec(reception->getStartTime());
    auto endTime = simsec(reception->getEndTime());
    Hz centerFrequency = narrowbandSignal->getCenterFrequency();
    Hz bandwidth = narrowbandSignal->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
//    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double meanSNIR = snir->getMean(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing mean SNIR: start = " << startPoint << ", end = " << endPoint << " -> mean SNIR = " << meanSNIR << endl;
    return meanSNIR;
}

double LayeredSnir::getMin() const
{
    if (std::isnan(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

double LayeredSnir::getMax() const
{
    if (std::isnan(maxSNIR))
        maxSNIR = computeMax();
    return maxSNIR;
}

double LayeredSnir::getMean() const
{
    if (std::isnan(meanSNIR))
        meanSNIR = computeMean();
    return meanSNIR;
}

const Ptr<const IFunction<double, Domain<simsec, Hz>>> LayeredSnir::getSnir() const
{
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const IDimensionalSignal *dimensionalSignal = check_and_cast<const IDimensionalSignal *>(reception->getAnalogModel());
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalSignal->getPower();
    return receptionPower->divide(noisePower);
}

} // namespace physicallayer

} // namespace inet

