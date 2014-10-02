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

#include "inet/physicallayer/dimensional/DimensionalSNIR.h"

namespace inet {

namespace physicallayer {

DimensionalSNIR::DimensionalSNIR(const DimensionalReception *reception, const DimensionalNoise *noise) :
    SNIRBase(reception, noise)
{
}

void DimensionalSNIR::printToStream(std::ostream& stream) const
{
    stream << "scalar SNIR";
}

double DimensionalSNIR::computeMin() const
{
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    EV_DEBUG << "Reception power begin " << endl;
    dimensionalReception->getPower()->print(EVSTREAM);
    EV_DEBUG << "Reception power end" << endl;
    const ConstMapping *snirMapping = MappingUtils::divide(*dimensionalReception->getPower(), *dimensionalNoise->getPower());
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Hz carrierFrequency = dimensionalReception->getCarrierFrequency();
    Hz bandwidth = dimensionalReception->getBandwidth();
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    EV_DEBUG << "SNIR begin " << endl;
    snirMapping->print(EVSTREAM);
    EV_DEBUG << "SNIR end" << endl;
    return MappingUtils::findMin(*snirMapping, start, end);
}

} // namespace physicallayer

} // namespace inet

