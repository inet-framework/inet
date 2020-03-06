//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalAnalogModelBase.h"

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/signal/PowerFunctions.h"

namespace inet {

namespace physicallayer {

void DimensionalAnalogModelBase::initialize(int stage)
{
    AnalogModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        attenuateWithCenterFrequency = par("attenuateWithCenterFrequency"); // TODO rename center
}

const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> DimensionalAnalogModelBase::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const IDimensionalSignal *dimensionalSignalAnalogModel = check_and_cast<const IDimensionalSignal *>(transmission->getAnalogModel());
    const INarrowbandSignal *narrowbandSignal = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const Coord& transmissionStartPosition = transmission->getStartPosition();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmissionStartPosition, arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmissionStartPosition, arrival->getStartOrientation());
    const auto& transmissionPowerFunction = dimensionalSignalAnalogModel->getPower();
    EV_TRACE << "Transmission power begin " << endl;
    EV_TRACE << *transmissionPowerFunction << endl;
    EV_TRACE << "Transmission power end" << endl;
    Point<simsec, Hz> propagationShift(simsec(arrival->getStartTime() - transmission->getStartTime()), Hz(0));
    const auto& propagatedTransmissionPowerFunction = makeShared<DomainShiftedFunction<WpHz, Domain<simsec, Hz>>>(transmissionPowerFunction, propagationShift);
    Ptr<const IFunction<double, Domain<simsec, Hz>>> attenuationFunction = makeShared<FrequencyDependentAttenuationFunction>(radioMedium, transmitterAntennaGain, receiverAntennaGain, transmissionStartPosition, receptionStartPosition);
    Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> receptionPower;
    if (attenuateWithCenterFrequency) {
        const auto& constantAttenuationFunction = makeShared<ConstantFunction<double, Domain<simsec, Hz>>>(attenuationFunction->getValue(Point<simsec, Hz>(simsec(0), narrowbandSignal->getCenterFrequency())));
        receptionPower = propagatedTransmissionPowerFunction->multiply(constantAttenuationFunction);
    }
    else {
        Hz lower = narrowbandSignal->getCenterFrequency() - narrowbandSignal->getBandwidth() / 2;
        Hz upper = narrowbandSignal->getCenterFrequency() + narrowbandSignal->getBandwidth() / 2;
        Hz step = narrowbandSignal->getBandwidth() / 10; // TODO: parameter for 10
        const auto& approximatedAttenuationFunction = makeShared<ApproximatedFunction<double, Domain<simsec, Hz>, 1, Hz>>(lower, upper, step, &AverageInterpolator<Hz, double>::singleton, attenuationFunction);
        receptionPower = propagatedTransmissionPowerFunction->multiply(approximatedAttenuationFunction);
    }
    EV_TRACE << "Reception power begin " << endl;
    EV_TRACE << *receptionPower << endl;
    EV_TRACE << "Reception power end" << endl;
    return receptionPower;
}

const INoise *DimensionalAnalogModelBase::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    std::vector<Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>> receptionPowers;
    const DimensionalNoise *dimensionalBackgroundNoise = check_and_cast_nullable<const DimensionalNoise *>(interference->getBackgroundNoise());
    if (dimensionalBackgroundNoise) {
        const auto& backgroundNoisePower = dimensionalBackgroundNoise->getPower();
        receptionPowers.push_back(backgroundNoisePower);
    }
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (const auto & interferingReception : *interferingReceptions) {
        auto dimensionalSignal = check_and_cast<const IDimensionalSignal *>(interferingReception->getAnalogModel());
        auto receptionPower = dimensionalSignal->getPower();
        receptionPowers.push_back(receptionPower);
        EV_TRACE << "Interference power begin " << endl;
        EV_TRACE << *receptionPower << endl;
        EV_TRACE << "Interference power end" << endl;
    }
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& noisePower = makeShared<SummedFunction<WpHz, Domain<simsec, Hz>>>(receptionPowers);
    EV_TRACE << "Noise power begin " << endl;
    EV_TRACE << *noisePower << endl;
    EV_TRACE << "Noise power end" << endl;
    const auto& bandpassFilter = makeShared<Boxcar2DFunction<double, simsec, Hz>>(simsec(listening->getStartTime()), simsec(listening->getEndTime()), centerFrequency - bandwidth / 2, centerFrequency + bandwidth / 2, 1);
    return new DimensionalNoise(listening->getStartTime(), listening->getEndTime(), centerFrequency, bandwidth, noisePower->multiply(bandpassFilter));
}

const INoise *DimensionalAnalogModelBase::computeNoise(const IReception *reception, const INoise *noise) const
{
    auto dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    auto dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& noisePower = makeShared<AddedFunction<WpHz, Domain<simsec, Hz>>>(dimensionalReception->getPower(), dimensionalNoise->getPower());
    return new DimensionalNoise(reception->getStartTime(), reception->getEndTime(), dimensionalReception->getCenterFrequency(), dimensionalReception->getBandwidth(), noisePower);
}

const ISnir *DimensionalAnalogModelBase::computeSNIR(const IReception *reception, const INoise *noise) const
{
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    return new DimensionalSnir(dimensionalReception, dimensionalNoise);
}

} // namespace physicallayer

} // namespace inet

