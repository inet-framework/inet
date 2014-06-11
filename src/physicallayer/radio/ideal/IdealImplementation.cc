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

Define_Module(IdealRadioSignalAttenuation);
Define_Module(IdealRadioSignalTransmitter);
Define_Module(IdealRadioSignalReceiver);

const IRadioSignalReception *IdealRadioSignalAttenuation::computeReception(const IRadio *receiverRadio, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiverRadio->getChannel();
    const IRadioSignalArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const IdealRadioSignalTransmission *idealTransmission = check_and_cast<const IdealRadioSignalTransmission *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const EulerAngles receptionStartOrientation = arrival->getStartOrientation();
    const EulerAngles receptionEndOrientation = arrival->getEndOrientation();
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    IdealRadioSignalReception::Power power;
    if (distance <= idealTransmission->getMaxCommunicationRange())
        power = IdealRadioSignalReception::POWER_RECEIVABLE;
    else if (distance <= idealTransmission->getMaxInterferenceRange())
        power = IdealRadioSignalReception::POWER_INTERFERING;
    else if (distance <= idealTransmission->getMaxDetectionRange())
        power = IdealRadioSignalReception::POWER_DETECTABLE;
    else
        power = IdealRadioSignalReception::POWER_UNDETECTABLE;
    return new IdealRadioSignalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, power);
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
    const simtime_t duration = (b(macFrame->getBitLength()) / bitrate).get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new IdealRadioSignalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, maxCommunicationRange, maxInterferenceRange, maxDetectionRange);
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
