//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ScalarAnalogModelBase.h"

#include "inet/common/geometry/common/Quaternion.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntennaGain.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

void ScalarAnalogModelBase::initialize(int stage)
{
    AnalogModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ignorePartialInterference = par("ignorePartialInterference");
    }
}

bool ScalarAnalogModelBase::areOverlappingBands(Hz centerFrequency1, Hz bandwidth1, Hz centerFrequency2, Hz bandwidth2) const
{
    return centerFrequency1 + bandwidth1 / 2 >= centerFrequency2 - bandwidth2 / 2 &&
           centerFrequency1 - bandwidth1 / 2 <= centerFrequency2 + bandwidth2 / 2;
}

W ScalarAnalogModelBase::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(transmission->getAnalogModel());
    const Coord& receptionStartPosition = arrival->getStartPosition();
    // TODO could be used for doppler shift? const Coord& receptionEndPosition = arrival->getEndPosition();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmission->getStartPosition(), arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmission->getStartPosition(), arrival->getStartOrientation());
    double pathLoss = radioMedium->getPathLoss()->computePathLoss(transmission, arrival);
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(narrowbandSignalAnalogModel->getCenterFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarSignalAnalogModel->getPower();
    ASSERT(!std::isnan(transmissionPower.get()));
    double gain = transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss;
    ASSERT(!std::isnan(gain));
    if (gain > 1.0) {
        EV_WARN << "Signal power attenuation is zero.\n";
        gain = 1.0;
    }
    return transmissionPower * gain;
}

void ScalarAnalogModelBase::addReception(const IReception *reception, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W>& powerChanges) const
{
    W power = check_and_cast<const IScalarSignal *>(reception->getAnalogModel())->getPower();
    simtime_t startTime = reception->getStartTime();
    simtime_t endTime = reception->getEndTime();
    std::map<simtime_t, W>::iterator itStartTime = powerChanges.find(startTime);
    if (itStartTime != powerChanges.end())
        itStartTime->second += power;
    else
        powerChanges.insert(std::pair<simtime_t, W>(startTime, power));
    std::map<simtime_t, W>::iterator itEndTime = powerChanges.find(endTime);
    if (itEndTime != powerChanges.end())
        itEndTime->second -= power;
    else
        powerChanges.insert(std::pair<simtime_t, W>(endTime, -power));
    if (reception->getStartTime() < noiseStartTime)
        noiseStartTime = reception->getStartTime();
    if (reception->getEndTime() > noiseEndTime)
        noiseEndTime = reception->getEndTime();
}

void ScalarAnalogModelBase::addNoise(const ScalarNoise *noise, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W>& powerChanges) const
{
    const auto& noisePowerFunction = noise->getPower();
    math::Point<simtime_t> startPoint(noise->getStartTime());
    math::Point<simtime_t> endPoint(noise->getEndTime());
    math::Interval<simtime_t> interval(startPoint, endPoint, 0b1, 0b0, 0b0);
    noisePowerFunction->partition(interval, [&] (const math::Interval<simtime_t>& i1, const math::IFunction<W, math::Domain<simtime_t>> *f) {
        auto lower = std::get<0>(i1.getLower());
        auto upper = std::get<0>(i1.getUpper());
        auto fc = check_and_cast<const math::ConstantFunction<W, math::Domain<simtime_t>> *>(f);
        std::map<simtime_t, W>::iterator it = powerChanges.find(lower);
        if (it != powerChanges.end())
            it->second += fc->getConstantValue();
        else
            powerChanges.insert(std::pair<simtime_t, W>(lower, fc->getConstantValue()));
        std::map<simtime_t, W>::iterator jt = powerChanges.find(upper);
        if (jt != powerChanges.end())
            jt->second -= fc->getConstantValue();
        else
            powerChanges.insert(std::pair<simtime_t, W>(upper, -fc->getConstantValue()));
    });
    if (noise->getStartTime() < noiseStartTime)
        noiseStartTime = noise->getStartTime();
    if (noise->getEndTime() > noiseEndTime)
        noiseEndTime = noise->getEndTime();
}

const INoise *ScalarAnalogModelBase::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz commonCenterFrequency = bandListening->getCenterFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> powerChanges;
    powerChanges[math::getLowerBound<simtime_t>()] = W(0);
    powerChanges[math::getUpperBound<simtime_t>()] = W(0);
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(signalAnalogModel);
        Hz signalCenterFrequency = narrowbandSignalAnalogModel->getCenterFrequency();
        Hz signalBandwidth = narrowbandSignalAnalogModel->getBandwidth();
        if (commonCenterFrequency == signalCenterFrequency && commonBandwidth >= signalBandwidth)
            addReception(reception, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, narrowbandSignalAnalogModel->getCenterFrequency(), narrowbandSignalAnalogModel->getBandwidth()))
            throw cRuntimeError("Partially interfering signals are not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    const ScalarNoise *scalarBackgroundNoise = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (commonCenterFrequency == scalarBackgroundNoise->getCenterFrequency() && commonBandwidth >= scalarBackgroundNoise->getBandwidth())
            addNoise(scalarBackgroundNoise, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, scalarBackgroundNoise->getCenterFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Partially interfering background noise is not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    EV_TRACE << "Noise power begin " << endl;
    W power = W(0);
    for (auto & it : powerChanges) {
        power += it.second;
        it.second = power;
        EV_TRACE << "Noise at " << it.first << " = " << power << endl;
    }
    EV_TRACE << "Noise power end" << endl;
    const auto& powerFunction = makeShared<math::Interpolated1DFunction<W, simtime_t>>(powerChanges, &math::LeftInterpolator<simtime_t, W>::singleton);
    return new ScalarNoise(noiseStartTime, noiseEndTime, commonCenterFrequency, commonBandwidth, powerFunction);
}

const INoise *ScalarAnalogModelBase::computeNoise(const IReception *reception, const INoise *noise) const
{
    auto scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> powerChanges;
    powerChanges[math::getLowerBound<simtime_t>()] = W(0);
    powerChanges[math::getUpperBound<simtime_t>()] = W(0);
    addReception(reception, noiseStartTime, noiseEndTime, powerChanges);
    addNoise(scalarNoise, noiseStartTime, noiseEndTime, powerChanges);
    W power = W(0);
    for (auto & it : powerChanges) {
        power += it.second;
        it.second = power;
    }
    const auto& powerFunction = makeShared<math::Interpolated1DFunction<W, simtime_t>>(powerChanges, &math::LeftInterpolator<simtime_t, W>::singleton);
    return new ScalarNoise(noiseStartTime, noiseEndTime, scalarNoise->getCenterFrequency(), scalarNoise->getBandwidth(), powerFunction);
}

const ISnir *ScalarAnalogModelBase::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new ScalarSnir(reception, noise);
}

} // namespace physicallayer

} // namespace inet

