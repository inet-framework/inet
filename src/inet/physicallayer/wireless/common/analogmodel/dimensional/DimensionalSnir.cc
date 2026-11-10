//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

DimensionalSnir::DimensionalSnir(const IReception *reception, const INoise *noise) :
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

Interval<simsec, Hz> DimensionalSnir::computeInterval(simtime_t startTime, simtime_t endTime) const
{
    auto dimensionalReception = check_and_cast<const DimensionalReceptionAnalogModel *>(reception->getAnalogModel());
    Hz centerFrequency = dimensionalReception->getCenterFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    Point<simsec, Hz> startPoint(simsec(startTime), centerFrequency - bandwidth / 2);
    Point<simsec, Hz> endPoint(simsec(endTime), centerFrequency + bandwidth / 2);
    // an interval of one point of time gives the SNIR at that time
    bool isPoint = startTime == endTime;
    return Interval<simsec, Hz>(startPoint, endPoint, 0b11, isPoint ? 0b10 : 0b00, isPoint ? 0b10 : 0b00);
}

double DimensionalSnir::computeMin(simtime_t startTime, simtime_t endTime) const
{
    EV_TRACE << "SNIR begin " << endl;
    EV_TRACE << *getSnir() << endl;
    EV_TRACE << "SNIR end" << endl;
    double minSNIR = getSnir()->getMin(computeInterval(startTime, endTime));
    EV_DEBUG << "Computing minimum SNIR" << EV_FIELD(startTime) << EV_FIELD(endTime) << EV_FIELD(minSNIR) << endl;
    return minSNIR;
}

double DimensionalSnir::computeMax(simtime_t startTime, simtime_t endTime) const
{
    double maxSNIR = getSnir()->getMax(computeInterval(startTime, endTime));
    EV_DEBUG << "Computing maximum SNIR" << EV_FIELD(startTime) << EV_FIELD(endTime) << EV_FIELD(maxSNIR) << endl;
    return maxSNIR;
}

double DimensionalSnir::computeMean(simtime_t startTime, simtime_t endTime) const
{
    double meanSNIR = getSnir()->getMean(computeInterval(startTime, endTime));
    EV_DEBUG << "Computing mean SNIR" << EV_FIELD(startTime) << EV_FIELD(endTime) << EV_FIELD(meanSNIR) << endl;
    return meanSNIR;
}

void DimensionalSnir::updateLastInterval(simtime_t startTime, simtime_t endTime) const
{
    if (startTime != lastStartTime || endTime != lastEndTime) {
        lastStartTime = startTime;
        lastEndTime = endTime;
        lastMinSNIR = NaN;
        lastMaxSNIR = NaN;
        lastMeanSNIR = NaN;
    }
}

double DimensionalSnir::getMin(simtime_t startTime, simtime_t endTime) const
{
    if (isWholeReception(startTime, endTime))
        return getMin();
    updateLastInterval(startTime, endTime);
    if (std::isnan(lastMinSNIR))
        lastMinSNIR = computeMin(startTime, endTime);
    return lastMinSNIR;
}

double DimensionalSnir::getMax(simtime_t startTime, simtime_t endTime) const
{
    if (isWholeReception(startTime, endTime))
        return getMax();
    updateLastInterval(startTime, endTime);
    if (std::isnan(lastMaxSNIR))
        lastMaxSNIR = computeMax(startTime, endTime);
    return lastMaxSNIR;
}

double DimensionalSnir::getMean(simtime_t startTime, simtime_t endTime) const
{
    if (isWholeReception(startTime, endTime))
        return getMean();
    updateLastInterval(startTime, endTime);
    if (std::isnan(lastMeanSNIR))
        lastMeanSNIR = computeMean(startTime, endTime);
    return lastMeanSNIR;
}

double DimensionalSnir::getMin() const
{
    if (std::isnan(minSNIR))
        minSNIR = computeMin(reception->getStartTime(), reception->getEndTime());
    return minSNIR;
}

double DimensionalSnir::getMax() const
{
    if (std::isnan(maxSNIR))
        maxSNIR = computeMax(reception->getStartTime(), reception->getEndTime());
    return maxSNIR;
}

double DimensionalSnir::getMean() const
{
    if (std::isnan(meanSNIR))
        meanSNIR = computeMean(reception->getStartTime(), reception->getEndTime());
    return meanSNIR;
}

const Ptr<const IFunction<double, Domain<simsec, Hz>>> DimensionalSnir::getSnir() const
{
    if (snir == nullptr) {
        const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
        auto dimensionalReception = check_and_cast<const DimensionalReceptionAnalogModel *>(reception->getAnalogModel());
        snir = dimensionalReception->getPower()->divide(dimensionalNoise->getPower());
    }
    return snir;
}

} // namespace physicallayer

} // namespace inet

