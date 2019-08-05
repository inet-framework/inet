//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/lora/loraphy/LoRaAnalogModel.h"

#include "inet/lora/loraphy/LoRaReceiver.h"
#include "inet/lora/loraphy/LoRaReception.h"
#include "inet/lora/loraphy/LoRaTransmission.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarAnalogModel.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarReception.h"
#include "inet/lora/loraphy/LoRaRadio.h"

namespace inet {

namespace lora {

Define_Module(LoRaAnalogModel);

std::ostream& LoRaAnalogModel::printToStream(std::ostream& stream, int level) const
{
    return stream << "LoRaAnalogModel";
}

const W LoRaAnalogModel::getBackgroundNoisePower(const LoRaBandListening *listening) const {
    //const LoRaBandListening *loRaListening = check_and_cast<const LoRaBandListening *>(listening);
    //Sensitivity values from Semtech SX1272/73 datasheet, table 10, Rev 3.1, March 2017
    W noisePower = W(math::dBmW2mW(-126.5) / 1000);
    if(listening->getLoRaSF() == 6)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-121) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-118) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-111) / 1000);
    }

    if (listening->getLoRaSF() == 7)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-124) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-122) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-116) / 1000);
    }

    if(listening->getLoRaSF() == 8)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-127) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-125) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-119) / 1000);
    }
    if(listening->getLoRaSF() == 9)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-130) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-128) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-122) / 1000);
    }
    if(listening->getLoRaSF() == 10)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-133) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-130) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-125) / 1000);
    }
    if(listening->getLoRaSF() == 11)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-135) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-132) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-128) / 1000);
    }
    if(listening->getLoRaSF() == 12)
    {
        if(listening->getLoRaBW() == Hz(125000)) noisePower = W(math::dBmW2mW(-137) / 1000);
        if(listening->getLoRaBW() == Hz(250000)) noisePower = W(math::dBmW2mW(-135) / 1000);
        if(listening->getLoRaBW() == Hz(500000)) noisePower = W(math::dBmW2mW(-129) / 1000);
    }
    return noisePower;
}

W LoRaAnalogModel::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
   // const IRadio *transmitterRadio = transmission->getTransmitter();
    const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(transmission->getAnalogModel());
    const Coord receptionEndPosition = arrival->getEndPosition();
    const Coord receptionStartPosition = arrival->getStartPosition();
    // TODO: could be used for doppler shift? const Coord receptionEndPosition = arrival->getEndPosition();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmission->getStartPosition(), arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmission->getStartPosition(), arrival->getStartOrientation());
    double pathLoss = radioMedium->getPathLoss()->computePathLoss(transmission, arrival);
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(narrowbandSignalAnalogModel->getCarrierFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarSignalAnalogModel->getPower();
    return transmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
 }

const IReception *LoRaAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const LoRaTransmission *loRaTransmission = check_and_cast<const LoRaTransmission *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion receptionEndOrientation = arrival->getEndOrientation();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    W receivedPower = computeReceptionPower(receiverRadio, transmission, arrival);
    Hz LoRaCF = loRaTransmission->getLoRaCF();
    int LoRaSF = loRaTransmission->getLoRaSF();
    Hz LoRaBW = loRaTransmission->getLoRaBW();
    int LoRaCR = loRaTransmission->getLoRaCR();
    return new LoRaReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, LoRaCF, LoRaBW, receivedPower, LoRaSF, LoRaCR);
}

const INoise *LoRaAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const LoRaBandListening *bandListening = check_and_cast<const LoRaBandListening *>(listening);
    Hz commonCarrierFrequency = bandListening->getLoRaCF();
    Hz commonBandwidth = bandListening->getLoRaBW();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(signalAnalogModel);
        const LoRaReception *loRaReception = check_and_cast<const LoRaReception *>(signalAnalogModel);
        Hz signalCarrierFrequency = loRaReception->getLoRaCF();
        Hz signalBandwidth = loRaReception->getLoRaBW();
        if((commonCarrierFrequency == signalCarrierFrequency && commonBandwidth == signalBandwidth))
        {
            const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(signalAnalogModel);
            W power = scalarSignalAnalogModel->getPower();
            simtime_t startTime = reception->getStartTime();
            simtime_t endTime = reception->getEndTime();
            if (startTime < noiseStartTime)
                noiseStartTime = startTime;
            if (endTime > noiseEndTime)
                noiseEndTime = endTime;
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
        }
        else if (areOverlappingBands(commonCarrierFrequency, commonBandwidth, narrowbandSignalAnalogModel->getCarrierFrequency(), narrowbandSignalAnalogModel->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }

    simtime_t startTime = listening->getStartTime();
    simtime_t endTime = listening->getEndTime();
    std::map<simtime_t, W> *backgroundNoisePowerChanges = new std::map<simtime_t, W>();
    const W noisePower = getBackgroundNoisePower(bandListening);
    backgroundNoisePowerChanges->insert(std::pair<simtime_t, W>(startTime, noisePower));
    backgroundNoisePowerChanges->insert(std::pair<simtime_t, W>(endTime, -noisePower));

    for (const auto & backgroundNoisePowerChange : *backgroundNoisePowerChanges) {
        std::map<simtime_t, W>::iterator jt = powerChanges->find(backgroundNoisePowerChange.first);
        if (jt != powerChanges->end())
            jt->second += backgroundNoisePowerChange.second;
        else
            powerChanges->insert(std::pair<simtime_t, W>(backgroundNoisePowerChange.first, backgroundNoisePowerChange.second));
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

const ISnir *LoRaAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new ScalarSnir(reception, noise);
}

} // namespace physicallayer

} // namespace inet

