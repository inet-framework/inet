//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"

namespace inet {

namespace physicallayer {

DimensionalSnir::DimensionalSnir(const DimensionalReception *reception, const DimensionalNoise *noise) :
    SnirBase(reception, noise),
    minSNIR(NaN),
    maxSNIR(NaN),
    meanSNIR(NaN)
{
}

std::ostream& DimensionalSnir::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalSnir";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(minSNIR);
    return SnirBase::printToStream(stream, level);
}

double DimensionalSnir::computeMin() const
{
    // TODO factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_TRACE << "Reception power begin " << endl;
    EV_TRACE << *dimensionalReception->getPower() << endl;
    EV_TRACE << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snir = receptionPower->divide(noisePower);
    simsec startTime = simsec(reception->getStartTime());
    simsec endTime = simsec(reception->getEndTime());
    Hz centerFrequency = dimensionalReception->getCenterFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double minSNIR = snir->getMin(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing minimum SNIR" << EV_FIELD(startPoint) << EV_FIELD(endPoint) << EV_FIELD(minSNIR) << endl;
    return minSNIR;
}

double DimensionalSnir::computeMax() const
{
    // TODO factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_DEBUG << "Reception power begin " << endl;
    EV_DEBUG << *dimensionalReception->getPower() << endl;
    EV_DEBUG << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snir = receptionPower->divide(noisePower);
    auto startTime = simsec(reception->getStartTime());
    auto endTime = simsec(reception->getEndTime());
    Hz centerFrequency = dimensionalReception->getCenterFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double maxSNIR = snir->getMax(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing maximum SNIR" << EV_FIELD(startPoint) << EV_FIELD(endPoint) << EV_FIELD(maxSNIR) << endl;
    return maxSNIR;
}

double DimensionalSnir::computeMean() const
{
    // TODO factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_TRACE << "Reception power begin " << endl;
    EV_TRACE << *dimensionalReception->getPower() << endl;
    EV_TRACE << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snir = receptionPower->divide(noisePower);
    auto startTime = simsec(reception->getStartTime());
    auto endTime = simsec(reception->getEndTime());
    Hz centerFrequency = dimensionalReception->getCenterFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    Point<simsec, Hz> startPoint(startTime, centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(endTime, centerFrequency + bandwidth / 2);
    EV_TRACE << "SNIR begin " << endl;
    EV_TRACE << *snir << endl;
    EV_TRACE << "SNIR end" << endl;
    double meanSNIR = snir->getMean(Interval<simsec, Hz>(startPoint, endPoint, 0b11, 0b00, 0b00));
    EV_DEBUG << "Computing mean SNIR" << EV_FIELD(startPoint) << EV_FIELD(endPoint) << EV_FIELD(meanSNIR) << endl;
    return meanSNIR;
}

double DimensionalSnir::getMin() const
{
    if (std::isnan(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

double DimensionalSnir::getMax() const
{
    if (std::isnan(maxSNIR))
        maxSNIR = computeMax();
    return maxSNIR;
}

double DimensionalSnir::getMean() const
{
    if (std::isnan(meanSNIR))
        meanSNIR = computeMean();
    return meanSNIR;
}

const Ptr<const IFunction<double, Domain<simsec, Hz>>> DimensionalSnir::getSnir() const
{
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    return receptionPower->divide(noisePower);
}

} // namespace physicallayer

} // namespace inet

