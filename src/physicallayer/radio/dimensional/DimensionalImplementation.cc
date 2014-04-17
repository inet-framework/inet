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

#include "DimensionalImplementation.h"
#include "IRadioChannel.h"

const IRadioSignalReception *DimensionalRadioSignalAttenuationBase::computeReception(const IRadio *receiverRadio, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiverRadio->getChannel();
    const IRadio *transmitterRadio = transmission->getTransmitter();
    const IRadioAntenna *receiverAntenna = receiverRadio->getAntenna();
    const IRadioAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const DimensionalRadioSignalTransmission *dimensionalTransmission = check_and_cast<const DimensionalRadioSignalTransmission *>(transmission);
    const IRadioSignalArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const Coord direction = receptionStartPosition - transmission->getStartPosition();
    // TODO: use antenna gains
    double transmitterAntennaGain = transmitterAntenna->getGain(direction);
    double receiverAntennaGain = receiverAntenna->getGain(direction);
    const IRadioSignalLoss *loss = computeLoss(transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition);
    const ConstMapping *lossFactor = check_and_cast<const DimensionalRadioSignalLoss *>(loss)->getFactor();
    const Mapping *transmissionPower = dimensionalTransmission->getPower();
    const Mapping *receptionPower = MappingUtils::multiply(*transmissionPower, *lossFactor, Argument::MappedZero);
    delete loss;
    return new DimensionalRadioSignalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionPower, dimensionalTransmission->getCarrierFrequency());
}

const IRadioSignalLoss *DimensionalRadioSignalFreeSpaceAttenuation::computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const
{
    const DimensionalRadioSignalTransmission *dimensionalTransmission = check_and_cast<const DimensionalRadioSignalTransmission *>(transmission);
    // TODO: iterate over frequencies in power mapping
    LossConstMapping *pathLoss = new LossConstMapping(DimensionSet::timeFreqDomain, computePathLoss(transmission, startTime, endTime, startPosition, endPosition, dimensionalTransmission->getCarrierFrequency()));
    return new DimensionalRadioSignalLoss(pathLoss);
}

const IRadioSignalNoise *DimensionalRadioBackgroundNoise::computeNoise(const IRadioSignalListening *listening) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalNoise *DimensionalRadioBackgroundNoise::computeNoise(const IRadioSignalReception *reception) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalListening *DimensionalRadioSignalReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

bool DimensionalRadioSignalReceiver::computeIsReceptionPossible(const IRadioSignalReception *reception) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalNoise *DimensionalRadioSignalReceiver::computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

double DimensionalRadioSignalReceiver::computeSNIRMin(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalListeningDecision *DimensionalRadioSignalReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    // TODO:
    throw cRuntimeError("Not yet implemented");
}

void DimensionalRadioSignalTransmitter::printToStream(std::ostream &stream) const
{
    stream << "dimensional radio signal transmitter, "
           << "bitrate = " << bitrate << ", "
           << "power = " <<  power << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const IRadioSignalTransmission *DimensionalRadioSignalTransmitter::createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const
{
    simtime_t duration = packet->getBitLength() / bitrate.get();
    simtime_t endTime = startTime + duration;
    IMobility *mobility = radio->getAntenna()->getMobility();
    Coord startPosition = mobility->getPosition(startTime);
    Coord endPosition = mobility->getPosition(endTime);
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, DimensionSet::timeFreqDomain, Mapping::LINEAR);
    Argument position(DimensionSet::timeFreqDomain);
    position.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth).get() / 2);
    position.setTime(startTime);
    powerMapping->setValue(position, power.get());
    position.setTime(endTime);
    powerMapping->setValue(position, power.get());
    position.setArgValue(Dimension::frequency, (carrierFrequency + bandwidth).get() / 2);
    position.setTime(startTime);
    powerMapping->setValue(position, power.get());
    position.setTime(endTime);
    powerMapping->setValue(position, power.get());
    return new DimensionalRadioSignalTransmission(radio, startTime, endTime, startPosition, endPosition, powerMapping, carrierFrequency);
}

void DimensionalRadioSignalReceiver::printToStream(std::ostream &stream) const
{
    stream << "dimensional radio signal receiver, energyDetection = " << energyDetection << ", " << "sensitivity = " <<  sensitivity;
}
