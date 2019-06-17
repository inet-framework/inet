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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/base/packetlevel/DimensionalAnalogModelBase.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/common/packetlevel/PowerFunction.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

void DimensionalAnalogModelBase::initialize(int stage)
{
    AnalogModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        attenuateWithCarrierFrequency = par("attenuateWithCarrierFrequency");
}

const Ptr<const math::IFunction<W, simtime_t, Hz>> DimensionalAnalogModelBase::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const DimensionalTransmission *dimensionalTransmission = check_and_cast<const DimensionalTransmission *>(transmission);
    const IDimensionalSignal *dimensionalSignalAnalogModel = check_and_cast<const IDimensionalSignal *>(transmission->getAnalogModel());
    const Coord transmissionStartPosition = transmission->getStartPosition();
    const Coord receptionStartPosition = arrival->getStartPosition();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmissionStartPosition, arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmissionStartPosition, arrival->getStartOrientation());
    auto transmissionPower = dimensionalSignalAnalogModel->getPower();
    EV_DEBUG << "Transmission power begin " << endl;
    EV_DEBUG << *transmissionPower << endl;
    EV_DEBUG << "Transmission power end" << endl;
    math::Point<simtime_t, Hz> propagationShift(arrival->getStartTime() - transmission->getStartTime(), Hz(0));
    Ptr<const math::IFunction<double, simtime_t, Hz>> attenuationFunction = makeShared<AttenuationFunction>(radioMedium, transmitterAntennaGain, receiverAntennaGain, transmissionStartPosition, receptionStartPosition);
    auto propagatedTransmissionPowerFunction = makeShared<math::ShiftFunction<W, simtime_t, Hz>>(transmissionPower, propagationShift);
    if (attenuateWithCarrierFrequency)
        attenuationFunction = makeShared<ConstantFunction<double, simtime_t, Hz>>(attenuationFunction->getValue(math::Point<simtime_t, Hz>(arrival->getStartTime(), dimensionalTransmission->getCarrierFrequency())));
    auto receptionPower = propagatedTransmissionPowerFunction->multiply(attenuationFunction);
    EV_DEBUG << "Reception power begin " << endl;
    EV_DEBUG << *receptionPower << endl;
    EV_DEBUG << "Reception power end" << endl;
    return receptionPower;
}

const INoise *DimensionalAnalogModelBase::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    std::vector<Ptr<const math::IFunction<W, simtime_t, Hz>>> receptionPowers;
    const DimensionalNoise *dimensionalBackgroundNoise = check_and_cast_nullable<const DimensionalNoise *>(interference->getBackgroundNoise());
    if (dimensionalBackgroundNoise) {
        const auto& backgroundNoisePower = dimensionalBackgroundNoise->getPower();
        receptionPowers.push_back(backgroundNoisePower);
    }
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (const auto & interferingReception : *interferingReceptions) {
        const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(interferingReception);
        auto receptionPower = dimensionalReception->getPower();
        receptionPowers.push_back(receptionPower);
        EV_DEBUG << "Interference power begin " << endl;
        EV_DEBUG << *receptionPower << endl;
        EV_DEBUG << "Interference power end" << endl;
    }
    const Ptr<const math::IFunction<W, simtime_t, Hz>>& noisePower = makeShared<math::SumFunction<W, simtime_t, Hz>>(receptionPowers);
    EV_TRACE << "Noise power begin " << endl;
    EV_TRACE << *noisePower << endl;
    EV_TRACE << "Noise power end" << endl;
    return new DimensionalNoise(listening->getStartTime(), listening->getEndTime(), carrierFrequency, bandwidth, noisePower);
}

const INoise *DimensionalAnalogModelBase::computeNoise(const IReception *reception, const INoise *noise) const
{
    auto dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    auto dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const Ptr<const math::IFunction<W, simtime_t, Hz>>& noisePower = makeShared<math::AdditionFunction<W, simtime_t, Hz>>(dimensionalReception->getPower(), dimensionalNoise->getPower());
    return new DimensionalNoise(reception->getStartTime(), reception->getEndTime(), dimensionalReception->getCarrierFrequency(), dimensionalReception->getBandwidth(), noisePower);
}

const ISnir *DimensionalAnalogModelBase::computeSNIR(const IReception *reception, const INoise *noise) const
{
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    return new DimensionalSnir(dimensionalReception, dimensionalNoise);
}

} // namespace physicallayer

} // namespace inet

