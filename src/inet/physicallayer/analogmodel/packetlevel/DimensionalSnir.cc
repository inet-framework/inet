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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSnir.h"

namespace inet {

namespace physicallayer {

DimensionalSnir::DimensionalSnir(const DimensionalReception *reception, const DimensionalNoise *noise) :
    SnirBase(reception, noise),
    minSNIR(NaN),
    maxSNIR(NaN)
{
}

std::ostream& DimensionalSnir::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalSnir";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", minSNIR = " << minSNIR;
    return SnirBase::printToStream(stream, level);
}

double DimensionalSnir::computeMin() const
{
    // TODO: factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_DEBUG << "Reception power begin " << endl;
    EV_DEBUG << dimensionalReception->getPower() << endl;
    EV_DEBUG << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snir = math::Function<W, simtime_t, Hz>::divide(receptionPower, noisePower);
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Hz carrierFrequency = dimensionalReception->getCarrierFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    math::Point<simtime_t, Hz> startPoint(startTime, carrierFrequency - bandwidth / 2);
    math::Point<simtime_t, Hz> endPoint(endTime, carrierFrequency + bandwidth / 2);
    EV_DEBUG << "SNIR begin " << endl;
    EV_DEBUG << snir << endl;
    EV_DEBUG << "SNIR end" << endl;
    double minSNIR = snir->getMin(math::Interval<simtime_t, Hz>(startPoint, endPoint));
    EV_DEBUG << "Computing minimum SNIR: start = " << startPoint << ", end = " << endPoint << " -> minimum SNIR = " << minSNIR << endl;
    delete snir;
    return minSNIR;
}

double DimensionalSnir::computeMax() const
{
    // TODO: factor out common part
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_DEBUG << "Reception power begin " << endl;
    EV_DEBUG << dimensionalReception->getPower() << endl;
    EV_DEBUG << "Reception power end" << endl;
    auto noisePower = dimensionalNoise->getPower();
    auto receptionPower = dimensionalReception->getPower();
    auto snir = math::Function<W, simtime_t, Hz>::divide(receptionPower, noisePower);
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Hz carrierFrequency = dimensionalReception->getCarrierFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    math::Point<simtime_t, Hz> startPoint(startTime, carrierFrequency - bandwidth / 2);
    math::Point<simtime_t, Hz> endPoint(endTime, carrierFrequency + bandwidth / 2);
    EV_DEBUG << "SNIR begin " << endl;
    EV_DEBUG << snir << endl;
    EV_DEBUG << "SNIR end" << endl;
    double maxSNIR = snir->getMax(math::Interval<simtime_t, Hz>(startPoint, endPoint));
    EV_DEBUG << "Computing maximum SNIR: start = " << startPoint << ", end = " << endPoint << " -> maximum SNIR = " << maxSNIR << endl;
    delete snir;
    return maxSNIR;
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

} // namespace physicallayer

} // namespace inet

