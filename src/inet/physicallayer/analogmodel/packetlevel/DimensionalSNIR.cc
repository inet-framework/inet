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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSNIR.h"

namespace inet {

namespace physicallayer {

DimensionalSNIR::DimensionalSNIR(const DimensionalReception *reception, const DimensionalNoise *noise) :
    SNIRBase(reception, noise),
    minSNIR(NaN)
{
}

std::ostream& DimensionalSNIR::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalSNIR";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", minSNIR = " << minSNIR;
    return SNIRBase::printToStream(stream, level);
}

double DimensionalSNIR::computeMin() const
{
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_DEBUG << "Reception power begin " << endl;
    dimensionalReception->getPower()->print(EVSTREAM);
    EV_DEBUG << "Reception power end" << endl;
    const ConstMapping *noisePower = dimensionalNoise->getPower();
    const ConstMapping *receptionPower = dimensionalReception->getPower();
    const ConstMapping *snirMapping = MappingUtils::divide(*receptionPower, *noisePower);
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Hz carrierFrequency = dimensionalReception->getCarrierFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    const DimensionSet& dimensions = receptionPower->getDimensionSet();
    Argument startArgument(dimensions);
    Argument endArgument(dimensions);
    if (dimensions.hasDimension(Dimension::time)) {
        startArgument.setTime(startTime);
        // NOTE: to exclude the moment where the reception power starts to be 0 again
        endArgument.setTime(MappingUtils::pre(endTime));
    }
    if (dimensions.hasDimension(Dimension::frequency)) {
        startArgument.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
        endArgument.setArgValue(Dimension::frequency, nexttoward((carrierFrequency + bandwidth / 2).get(), 0));
    }
    EV_DEBUG << "SNIR begin " << endl;
    snirMapping->print(EVSTREAM);
    EV_DEBUG << "SNIR end" << endl;
    double minSNIR = MappingUtils::findMin(*snirMapping, startArgument, endArgument);
    EV_DEBUG << "Computing minimum SNIR: start = " << startArgument << ", end = " << endArgument << " -> minimum SNIR = " << minSNIR << endl;
    delete snirMapping;
    return minSNIR;
}

double DimensionalSNIR::getMin() const
{
    if (std::isnan(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

} // namespace physicallayer

} // namespace inet

