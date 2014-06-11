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
#include "GenericImplementation.h"
#include "Modulation.h"

Define_Module(DimensionalRadioSignalAttenuation);
Define_Module(DimensionalRadioBackgroundNoise);
Define_Module(DimensionalRadioSignalReceiver);
Define_Module(DimensionalRadioSignalTransmitter);

static ConstMapping *createMapping(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power)
{
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, DimensionSet::timeFreqDomain, Mapping::LINEAR);
    Argument position(DimensionSet::timeFreqDomain);
    position.setTime(startTime);
    position.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    powerMapping->setValue(position, power.get());
    position.setTime(endTime);
    powerMapping->setValue(position, power.get());
    position.setTime(startTime);
    position.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    powerMapping->setValue(position, power.get());
    position.setTime(endTime);
    powerMapping->setValue(position, power.get());
    return powerMapping;
}

W DimensionalRadioSignalReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    return W(MappingUtils::findMin(*power, start, end));
}

W DimensionalRadioSignalNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    return W(MappingUtils::findMax(*power));// TODO: W(MappingUtils::findMax(*power), start, end));
}

const IRadioSignalReception *DimensionalRadioSignalAttenuation::computeReception(const IRadio *receiverRadio, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiverRadio->getChannel();
    const IRadio *transmitterRadio = transmission->getTransmitter();
    const IRadioAntenna *receiverAntenna = receiverRadio->getAntenna();
    const IRadioAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const DimensionalRadioSignalTransmission *dimensionalTransmission = check_and_cast<const DimensionalRadioSignalTransmission *>(transmission);
    const IRadioSignalArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t transmissionStartTime = transmission->getStartTime();
    const simtime_t transmissionEndTime = transmission->getEndTime();
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const EulerAngles receptionStartOrientation = arrival->getStartOrientation();
    const EulerAngles receptionEndOrientation = arrival->getEndOrientation();
    const EulerAngles transmissionDirection = computeTransmissionDirection(transmission, arrival);
    const EulerAngles transmissionAntennaDirection = transmission->getStartOrientation() - transmissionDirection;
    const EulerAngles receptionAntennaDirection = transmissionDirection - arrival->getStartOrientation();
    double transmitterAntennaGain = transmitterAntenna->computeGain(transmissionAntennaDirection);
    double receiverAntennaGain = receiverAntenna->computeGain(receptionAntennaDirection);
    m distance = m(receptionStartPosition.distance(transmission->getStartPosition()));
    mps propagationSpeed = channel->getPropagation()->getPropagationSpeed();
    const ConstMapping *transmissionPower = dimensionalTransmission->getPower();
    std::cout << "Transmission power begin " << endl;
    transmissionPower->print(std::cout);
    std::cout << "Transmission power end" << endl;
    ConstMappingIterator *it = transmissionPower->createConstIterator();
    Mapping *receptionPower = MappingUtils::createMapping(Argument::MappedZero, DimensionSet::timeFreqDomain, Mapping::LINEAR);
    while (true)
    {
        const Argument& position = it->getPosition();
        Hz carrierFrequency = Hz(position.getArgValue(Dimension::frequency));
        double pathLossFactor = channel->getPathLoss()->computePathLoss(propagationSpeed, carrierFrequency, distance);
        W frequencyTransmissionPower = W(it->getValue());
        W frequencyReceptionPower = frequencyTransmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLossFactor);
        Argument receptionPosition = position;
        double alpha = (position.getTime() - transmissionStartTime).dbl() / (transmissionEndTime - transmissionStartTime).dbl();
        receptionPosition.setTime(receptionStartTime + alpha * (receptionEndTime - receptionStartTime).dbl());
        receptionPower->setValue(receptionPosition, frequencyReceptionPower.get());
        if (it->hasNext())
            it->next();
        else
            break;
    }
    std::cout << "Reception power begin " << endl;
    receptionPower->print(std::cout);
    std::cout << "Reception power end" << endl;
    return new DimensionalRadioSignalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, dimensionalTransmission->getCarrierFrequency(), dimensionalTransmission->getBandwidth(), receptionPower);
}

void DimensionalRadioBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        power = mW(FWMath::dBm2mW(par("power")));
    }
}

const IRadioSignalNoise *DimensionalRadioBackgroundNoise::computeNoise(const IRadioSignalListening *listening) const
{
    const BandRadioSignalListening *bandListening = check_and_cast<const BandRadioSignalListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    const ConstMapping *powerMapping = createMapping(startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalRadioSignalNoise(startTime, endTime, carrierFrequency, bandwidth, powerMapping);
}

void DimensionalRadioSignalTransmitter::initialize(int stage)
{
    FlatRadioSignalTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        bitrate = bps(par("bitrate"));
        power = W(par("power"));
    }
}

void DimensionalRadioSignalTransmitter::printToStream(std::ostream &stream) const
{
    stream << "dimensional radio signal transmitter, "
           << "bitrate = " << bitrate << ", "
           << "power = " <<  power << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const IRadioSignalTransmission *DimensionalRadioSignalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const simtime_t duration = macFrame->getBitLength() / bitrate.get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createMapping(startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalRadioSignalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, bitrate, powerMapping);
}


void DimensionalRadioSignalReceiver::initialize(int stage)
{
    SNIRRadioSignalReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        energyDetection = mW(FWMath::dBm2mW(par("energyDetection")));
        sensitivity = mW(FWMath::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        const char *modulationName = par("modulation");
        if (strcmp(modulationName, "NULL")==0)
            modulation = new NullModulation();
        else if (strcmp(modulationName, "BPSK")==0)
            modulation = new BPSKModulation();
        else if (strcmp(modulationName, "16-QAM")==0)
            modulation = new QAM16Modulation();
        else if (strcmp(modulationName, "256-QAM")==0)
            modulation = new QAM256Modulation();
        else
            throw cRuntimeError(this, "Unknown modulation '%s'", modulationName);
    }
}

void DimensionalRadioSignalReceiver::printToStream(std::ostream &stream) const
{
    stream << "dimensional radio signal receiver, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " <<  sensitivity;
}

const IRadioSignalListening *DimensionalRadioSignalReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandRadioSignalListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

bool DimensionalRadioSignalReceiver::computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const
{
    // TODO: check if modulation matches?
    const DimensionalRadioSignalTransmission *dimensionalTransmission = check_and_cast<const DimensionalRadioSignalTransmission *>(transmission);
    if (carrierFrequency == dimensionalTransmission->getCarrierFrequency() && bandwidth == dimensionalTransmission->getBandwidth())
        return true;
    else if (areOverlappingBands(carrierFrequency, bandwidth, dimensionalTransmission->getCarrierFrequency(), dimensionalTransmission->getBandwidth()))
        throw cRuntimeError("Overlapping bands are not supported");
    else
        return false;
}

// TODO: this is not purely functional, see interface comment
bool DimensionalRadioSignalReceiver::computeIsReceptionPossible(const IRadioSignalReception *reception) const
{
    const DimensionalRadioSignalReception *dimensionalReception = check_and_cast<const DimensionalRadioSignalReception *>(reception);
    if (carrierFrequency == dimensionalReception->getCarrierFrequency() && bandwidth == dimensionalReception->getBandwidth())
        return dimensionalReception->computeMinPower(reception->getStartTime(), reception->getEndTime()) >= sensitivity;
    else if (areOverlappingBands(carrierFrequency, bandwidth, dimensionalReception->getCarrierFrequency(), dimensionalReception->getBandwidth()))
        throw cRuntimeError("Overlapping bands are not supported");
    else
        return false;
}

const IRadioSignalNoise *DimensionalRadioSignalReceiver::computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const
{
    std::vector<ConstMapping *> receptionPowers;
    const DimensionalRadioSignalNoise *dimensionalBackgroundNoise = check_and_cast<const DimensionalRadioSignalNoise *>(backgroundNoise);
    receptionPowers.push_back(const_cast<ConstMapping *>(dimensionalBackgroundNoise->getPower()));
    for (std::vector<const IRadioSignalReception *>::const_iterator it = receptions->begin(); it != receptions->end(); it++)
    {
        const DimensionalRadioSignalReception *reception = check_and_cast<const DimensionalRadioSignalReception *>(*it);
        receptionPowers.push_back(const_cast<ConstMapping *>(reception->getPower()));
    }
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    const BandRadioSignalListening *bandListening = check_and_cast<const BandRadioSignalListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    ConstMapping *listeningMapping = createMapping(startTime, endTime, carrierFrequency, bandwidth, W(0));

    // TODO: delme
    for (std::vector<ConstMapping *>::const_iterator it = receptionPowers.begin(); it != receptionPowers.end(); it++)
    {
        std::cout << "Interference power begin " << *it << endl;
        (*it)->print(std::cout);
        std::cout << "Interference power end" << endl;
    }

    ConcatConstMapping<std::plus<double> > *noisePower = new ConcatConstMapping<std::plus<double> >(listeningMapping, receptionPowers.begin(), receptionPowers.end(), false, Argument::MappedZero);
    std::cout << "Noise power begin " << endl;
    noisePower->print(std::cout);
    std::cout << "Noise power end" << endl;
    return new DimensionalRadioSignalNoise(listening->getStartTime(), listening->getEndTime(), carrierFrequency, bandwidth, noisePower);
}

const IRadioSignalListeningDecision *DimensionalRadioSignalReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const IRadioSignalNoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    const DimensionalRadioSignalNoise *dimensionalNoise = check_and_cast<const DimensionalRadioSignalNoise *>(noise);
    W maxPower = dimensionalNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    delete noise;
    return new RadioSignalListeningDecision(listening, maxPower >= energyDetection);
}

// TODO: this is not purely functional, see interface comment
bool DimensionalRadioSignalReceiver::computeHasBitError(double minSNIR, int bitLength, double bitrate) const
{
    double ber = modulation->calculateBER(minSNIR, bandwidth.get(), bitrate);
    if (ber == 0.0)
        return false;
    else
    {
        double pErrorless = pow(1.0 - ber, bitLength);
        return dblrand() > pErrorless;
    }
}

bool DimensionalRadioSignalReceiver::computeIsReceptionSuccessful(const IRadioSignalReception *reception, const RadioReceptionIndication *indication) const
{
    const DimensionalRadioSignalTransmission *dimensionalTransmission = check_and_cast<const DimensionalRadioSignalTransmission *>(reception->getTransmission());
    return SNIRRadioSignalReceiverBase::computeIsReceptionSuccessful(reception, indication) &&
           !computeHasBitError(indication->getMinSNIR(), dimensionalTransmission->getPayloadBitLength(), dimensionalTransmission->getBitrate().get());
}

const IRadioSignalReceptionDecision *DimensionalRadioSignalReceiver::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const BandRadioSignalListening *bandListening = check_and_cast<const BandRadioSignalListening *>(listening);
    const DimensionalRadioSignalReception *dimensionalReception = check_and_cast<const DimensionalRadioSignalReception *>(reception);
    if (bandListening->getCarrierFrequency() == dimensionalReception->getCarrierFrequency() && bandListening->getBandwidth() == dimensionalReception->getBandwidth())
        return SNIRRadioSignalReceiverBase::computeReceptionDecision(listening, reception, interferingReceptions, backgroundNoise);
    else if (areOverlappingBands(bandListening->getCarrierFrequency(), bandListening->getBandwidth(), dimensionalReception->getCarrierFrequency(), dimensionalReception->getBandwidth()))
        throw cRuntimeError("Overlapping bands are not supported");
    else
        return new RadioSignalReceptionDecision(reception, new RadioReceptionIndication(), false, false, false);
}

double DimensionalRadioSignalReceiver::computeMinSNIR(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const
{
    const DimensionalRadioSignalNoise *dimensionalNoise = check_and_cast<const DimensionalRadioSignalNoise *>(noise);
    const DimensionalRadioSignalReception *dimensionalReception = check_and_cast<const DimensionalRadioSignalReception *>(reception);
    std::cout << "Reception power begin " << endl;
    dimensionalReception->getPower()->print(std::cout);
    std::cout << "Reception power end" << endl;
    const ConstMapping *snirMapping = MappingUtils::divide(*dimensionalReception->getPower(), *dimensionalNoise->getPower());
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    std::cout << "TIMES" << startTime << " " << endTime << endl;
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    std::cout << "SNIR begin " << endl;
    snirMapping->print(std::cout);
    std::cout << "SNIR end" << endl;
    double snirMin = MappingUtils::findMin(*snirMapping, start, end);
    std::cout << "XXX" << snirMin << endl;
    return snirMin;
}
