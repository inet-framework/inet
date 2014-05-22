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

#include "IdealImplementation.h"
#include "GenericImplementation.h"
#include "IRadioChannel.h"

Define_Module(IdealRadioSignalFreeSpaceAttenuation);
Define_Module(IdealRadioSignalTransmitter);
Define_Module(IdealRadioSignalReceiver);

const IRadioSignalReception *IdealRadioSignalAttenuationBase::computeReception(const IRadio *receiverRadio, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiverRadio->getChannel();
    const IRadioSignalArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const IRadioSignalLoss *loss = computeLoss(transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition);
    IdealRadioSignalLoss::Factor lossFactor = check_and_cast<const IdealRadioSignalLoss *>(loss)->getFactor();
    IdealRadioSignalReception::Power power;
    switch (lossFactor)
    {
        case IdealRadioSignalLoss::FACTOR_WITHIN_COMMUNICATION_RANGE:
            power = IdealRadioSignalReception::POWER_RECEIVABLE; break;
        case IdealRadioSignalLoss::FACTOR_WITHIN_INTERFERENCE_RANGE:
            power = IdealRadioSignalReception::POWER_INTERFERING; break;
        case IdealRadioSignalLoss::FACTOR_WITHIN_DETECTION_RANGE:
            power = IdealRadioSignalReception::POWER_DETECTABLE; break;
        case IdealRadioSignalLoss::FACTOR_OUT_OF_DETECTION_RANGE:
            power = IdealRadioSignalReception::POWER_UNDETECTABLE; break;
        default:
            throw cRuntimeError("Unknown attenuation factor");
    }
    delete loss;
    return new IdealRadioSignalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, power);
}

const IRadioSignalLoss *IdealRadioSignalFreeSpaceAttenuation::computeLoss(const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    const IdealRadioSignalTransmission *idealTransmission = check_and_cast<const IdealRadioSignalTransmission *>(transmission);
    m distance = m(transmission->getStartPosition().distance(startPosition));
    IdealRadioSignalLoss::Factor factor;
    if (distance <= idealTransmission->getMaxCommunicationRange())
        factor = IdealRadioSignalLoss::FACTOR_WITHIN_COMMUNICATION_RANGE;
    else if (distance <= idealTransmission->getMaxInterferenceRange())
        factor = IdealRadioSignalLoss::FACTOR_WITHIN_INTERFERENCE_RANGE;
    else if (distance <= idealTransmission->getMaxDetectionRange())
        factor = IdealRadioSignalLoss::FACTOR_WITHIN_DETECTION_RANGE;
    else
        factor = IdealRadioSignalLoss::FACTOR_OUT_OF_DETECTION_RANGE;
    return new IdealRadioSignalLoss(factor);
}

void IdealRadioSignalTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        bitrate = bps(par("bitrate"));
        maxCommunicationRange = m(par("maxCommunicationRange"));
        maxInterferenceRange = m(par("maxInterferenceRange"));
        maxDetectionRange = m(par("maxDetectionRange"));
    }
}

void IdealRadioSignalTransmitter::printToStream(std::ostream &stream) const
{
    stream << "ideal radio signal transmitter, bitrate = " << bitrate << ", "
           << "maxCommunicationRange = " << maxCommunicationRange << ", "
           << "maxInterferenceRange = " << maxInterferenceRange << ", "
           << "maxDetectionRange = " << maxDetectionRange;
}

const IRadioSignalTransmission *IdealRadioSignalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    simtime_t duration = (b(macFrame->getBitLength()) / bitrate).get();
    simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    Coord startPosition = mobility->getPosition(startTime);
    Coord endPosition = mobility->getPosition(endTime);
    return new IdealRadioSignalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, maxCommunicationRange, maxInterferenceRange, maxDetectionRange);
}

void IdealRadioSignalReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ignoreInterference = par("ignoreInterference");
    }
}

bool IdealRadioSignalReceiver::computeIsReceptionPossible(const IRadioSignalReception *reception) const
{
    const IdealRadioSignalReception::Power power = check_and_cast<const IdealRadioSignalReception *>(reception)->getPower();
    return power == IdealRadioSignalReception::POWER_RECEIVABLE;
}

bool IdealRadioSignalReceiver::computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const
{
    if (ignoreInterference)
        return computeIsReceptionPossible(reception);
    else
        return RadioSignalReceiverBase::computeIsReceptionAttempted(reception, interferingReceptions);
}

void IdealRadioSignalReceiver::printToStream(std::ostream &stream) const
{
    stream << "ideal radio signal receiver, " << (ignoreInterference ? "ignore interference" : "compute interference");
}

const IRadioSignalListening *IdealRadioSignalReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new IdealRadioSignalListening(radio, startTime, endTime, startPosition, endPosition);
}

const IRadioSignalListeningDecision *IdealRadioSignalReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
    {
        const IRadioSignalReception *interferingReception = *it;
        IdealRadioSignalReception::Power interferingPower = check_and_cast<const IdealRadioSignalReception *>(interferingReception)->getPower();
        if (interferingPower != IdealRadioSignalReception::POWER_UNDETECTABLE)
            return new RadioSignalListeningDecision(listening, true);
    }
    return new RadioSignalListeningDecision(listening, false);
}

const IRadioSignalReceptionDecision *IdealRadioSignalReceiver::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const IdealRadioSignalReception::Power power = check_and_cast<const IdealRadioSignalReception *>(reception)->getPower();
    RadioReceptionIndication *indication = new RadioReceptionIndication();
    if (power == IdealRadioSignalReception::POWER_RECEIVABLE)
    {
        if (ignoreInterference)
            return new RadioSignalReceptionDecision(reception, indication, true, true, true);
        else
        {
            for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
            {
                const IRadioSignalReception *interferingReception = *it;
                IdealRadioSignalReception::Power interferingPower = check_and_cast<const IdealRadioSignalReception *>(interferingReception)->getPower();
                if (interferingPower == IdealRadioSignalReception::POWER_RECEIVABLE || interferingPower == IdealRadioSignalReception::POWER_INTERFERING)
                    return new RadioSignalReceptionDecision(reception, indication, true, true, false);
            }
            return new RadioSignalReceptionDecision(reception, indication, true, true, true);
        }
    }
    else
        return new RadioSignalReceptionDecision(reception, indication, false, false, false);
}
