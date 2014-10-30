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

#include "inet/physicallayer/dimensional/DimensionalReceiver.h"
#include "inet/physicallayer/dimensional/DimensionalReception.h"
#include "inet/physicallayer/dimensional/DimensionalNoise.h"
#include "inet/physicallayer/dimensional/DimensionalSNIR.h"
#include "inet/physicallayer/common/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalReceiver);

DimensionalReceiver::DimensionalReceiver() :
    FlatReceiverBase()
{
}

void DimensionalReceiver::printToStream(std::ostream& stream) const
{
    stream << "DimensionalReceiver, ";
    FlatReceiverBase::printToStream(stream);
}

const INoise *DimensionalReceiver::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    std::vector<ConstMapping *> receptionPowers;
    const DimensionalNoise *dimensionalBackgroundNoise = dynamic_cast<const DimensionalNoise *>(interference->getBackgroundNoise());
    if (dimensionalBackgroundNoise) {
        const ConstMapping *backgroundNoisePower = dimensionalBackgroundNoise->getPower();
        if (backgroundNoisePower->getDimensionSet().hasDimension(Dimension::frequency) || (carrierFrequency == dimensionalBackgroundNoise->getCarrierFrequency() && bandwidth == dimensionalBackgroundNoise->getBandwidth()))
            receptionPowers.push_back(const_cast<ConstMapping *>(backgroundNoisePower));
    }
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (std::vector<const IReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++) {
        const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(*it);
        const ConstMapping *receptionPower = dimensionalReception->getPower();
        if (receptionPower->getDimensionSet().hasDimension(Dimension::frequency) || (carrierFrequency == dimensionalReception->getCarrierFrequency() && bandwidth == dimensionalReception->getBandwidth())) {
            receptionPowers.push_back(const_cast<ConstMapping *>(receptionPower));
            EV_DEBUG << "Interference power begin " << endl;
            dimensionalReception->getPower()->print(EVSTREAM);
            EV_DEBUG << "Interference power end" << endl;
        }
    }
    DimensionSet dimensions = receptionPowers[0]->getDimensionSet();
    if (!dimensions.hasDimension(Dimension::time))
        dimensions.addDimension(Dimension::time);
    ConstMapping *listeningMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, Mapping::STEPS);
    ConcatConstMapping<std::plus<double> > *noisePower = new ConcatConstMapping<std::plus<double> >(listeningMapping, receptionPowers.begin(), receptionPowers.end(), false, Argument::MappedZero);
    EV_DEBUG << "Noise power begin " << endl;
    noisePower->print(EVSTREAM);
    EV_DEBUG << "Noise power end" << endl;
    return new DimensionalNoise(listening->getStartTime(), listening->getEndTime(), carrierFrequency, bandwidth, noisePower);
}

const ISNIR *DimensionalReceiver::computeSNIR(const IReception *reception, const INoise *noise) const
{
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    return new DimensionalSNIR(dimensionalReception, dimensionalNoise);
}

} // namespace physicallayer

} // namespace inet

