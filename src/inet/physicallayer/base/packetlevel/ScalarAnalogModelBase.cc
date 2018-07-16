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

#include "inet/common/geometry/common/Quaternion.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/base/packetlevel/ScalarAnalogModelBase.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/contract/packetlevel/IAntennaGain.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

bool ScalarAnalogModelBase::areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const
{
    return carrierFrequency1 + bandwidth1 / 2 >= carrierFrequency2 - bandwidth2 / 2 &&
           carrierFrequency1 - bandwidth1 / 2 <= carrierFrequency2 + bandwidth2 / 2;
}

W ScalarAnalogModelBase::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(transmission->getAnalogModel());
    const Coord receptionStartPosition = arrival->getStartPosition();
    // TODO: could be used for doppler shift? const Coord receptionEndPosition = arrival->getEndPosition();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmission->getStartPosition(), arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmission->getStartPosition(), arrival->getStartOrientation());
    double pathLoss = radioMedium->getPathLoss()->computePathLoss(transmission, arrival);
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(narrowbandSignalAnalogModel->getCarrierFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarSignalAnalogModel->getPower();
    return transmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
}

void ScalarAnalogModelBase::addReception(const ScalarReception *reception, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W> *powerChanges) const
{
    W power = reception->getPower();
    simtime_t startTime = reception->getStartTime();
    simtime_t endTime = reception->getEndTime();
    std::map<simtime_t, W>::iterator itStartTime = powerChanges->find(startTime);
    if (itStartTime != powerChanges->end())
        itStartTime->second += power;
    else
        powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
    std::map<simtime_t, W>::iterator itEndTime = powerChanges->find(endTime);
    if (itEndTime != powerChanges->end())
        itEndTime->second -= power;
    else
        powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
    if (reception->getStartTime() < noiseStartTime)
        noiseStartTime = reception->getStartTime();
    if (reception->getEndTime() > noiseEndTime)
        noiseEndTime = reception->getEndTime();
}

void ScalarAnalogModelBase::addNoise(const ScalarNoise *noise, simtime_t& noiseStartTime, simtime_t& noiseEndTime, std::map<simtime_t, W> *powerChanges) const
{
    const std::map<simtime_t, W> *noisePowerChanges = noise->getPowerChanges();
    for (const auto & noisePowerChange : *noisePowerChanges) {
        std::map<simtime_t, W>::iterator jt = powerChanges->find(noisePowerChange.first);
        if (jt != powerChanges->end())
            jt->second += noisePowerChange.second;
        else
            powerChanges->insert(std::pair<simtime_t, W>(noisePowerChange.first, noisePowerChange.second));
    }
    if (noise->getStartTime() < noiseStartTime)
        noiseStartTime = noise->getStartTime();
    if (noise->getEndTime() > noiseEndTime)
        noiseEndTime = noise->getEndTime();
}

const INoise *ScalarAnalogModelBase::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz commonCarrierFrequency = bandListening->getCarrierFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(signalAnalogModel);
        Hz signalCarrierFrequency = narrowbandSignalAnalogModel->getCarrierFrequency();
        Hz signalBandwidth = narrowbandSignalAnalogModel->getBandwidth();
        if (commonCarrierFrequency == signalCarrierFrequency && commonBandwidth == signalBandwidth)
            addReception(check_and_cast<const ScalarReception *>(reception), noiseStartTime, noiseEndTime, powerChanges);
        else if (areOverlappingBands(commonCarrierFrequency, commonBandwidth, narrowbandSignalAnalogModel->getCarrierFrequency(), narrowbandSignalAnalogModel->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    const ScalarNoise *scalarBackgroundNoise = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (commonCarrierFrequency == scalarBackgroundNoise->getCarrierFrequency() && commonBandwidth == scalarBackgroundNoise->getBandwidth())
            addNoise(scalarBackgroundNoise, noiseStartTime, noiseEndTime, powerChanges);
        else if (areOverlappingBands(commonCarrierFrequency, commonBandwidth, scalarBackgroundNoise->getCarrierFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    EV_TRACE << "Noise power begin " << endl;
    W noise = W(0);
    for (std::map<simtime_t, W>::const_iterator it = powerChanges->begin(); it != powerChanges->end(); it++) {
        noise += it->second;
        EV_TRACE << "Noise at " << it->first << " = " << noise << endl;
    }
    EV_TRACE << "Noise power end" << endl;
    return new ScalarNoise(noiseStartTime, noiseEndTime, commonCarrierFrequency, commonBandwidth, powerChanges);
}

const INoise *ScalarAnalogModelBase::computeNoise(const IReception *reception, const INoise *noise) const
{
    auto scalarReception = check_and_cast<const ScalarReception *>(reception);
    auto scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    addReception(scalarReception, noiseStartTime, noiseEndTime, powerChanges);
    addNoise(scalarNoise, noiseStartTime, noiseEndTime, powerChanges);
    return new ScalarNoise(noiseStartTime, noiseEndTime, scalarNoise->getCarrierFrequency(), scalarNoise->getBandwidth(), powerChanges);
}

const ISnir *ScalarAnalogModelBase::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new ScalarSnir(reception, noise);
}

} // namespace physicallayer

} // namespace inet

