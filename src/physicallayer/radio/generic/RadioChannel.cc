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

#include "Radio.h"
#include "RadioChannel.h"
// TODO: should not be here
#include "ScalarImplementation.h"

Define_Module(RadioChannel);

RadioChannel::~RadioChannel()
{
    delete propagation;
    delete attenuation;
    delete backgroundNoise;
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
        delete *it;
    for (std::vector<std::vector<const IRadioSignalArrival *> >::const_iterator it = arrivals.begin(); it != arrivals.end(); it++)
        for (std::vector<const IRadioSignalArrival *>::const_iterator jt = it->begin(); jt != it->end(); jt++)
            delete *jt;
}

void RadioChannel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        minInterferenceTime = computeMinInterferenceTime();
        maxTransmissionDuration = computeMaxTransmissionDuration();
        // TODO: use computeMaxCommunicationRange();
        maxCommunicationRange = computeMaxInterferenceRange();
        maxInterferenceRange = computeMaxInterferenceRange();
        propagation = check_and_cast<IRadioSignalPropagation *>(getSubmodule("propagation"));
        attenuation = check_and_cast<IRadioSignalAttenuation *>(getSubmodule("attenuation"));
        backgroundNoise = dynamic_cast<IRadioBackgroundNoise *>(getSubmodule("backgroundNoise"));
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio channel initialized with"
                 << " minimum interference time = " << minInterferenceTime << " s"
                 << ", maximum transmission duration = " << maxTransmissionDuration << " s"
                 << ", maximum communication range = " << maxCommunicationRange
                 << ", maximum interference range = " << maxInterferenceRange
                 << ", " << propagation << ", " << attenuation << ", " << backgroundNoise << endl;
    }
}

m RadioChannel::computeMaxRange(W maxPower, W minPower) const
{
    double alpha = par("alpha");
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    m waveLength = mps(SPEED_OF_LIGHT) / carrierFrequency;
    double minFactor = (minPower / maxPower).get();
    return waveLength / pow(minFactor * 16.0 * M_PI * M_PI, 1.0 / alpha);
}

m RadioChannel::computeMaxCommunicationRange() const
{
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    if (isNaN(carrierFrequency.get()))
        return m(par("maxCommunicationRange"));
    else
        return computeMaxRange(W(par("maxTransmissionPower")), mW(FWMath::dBm2mW(par("minReceptionPower"))));
}

m RadioChannel::computeMaxInterferenceRange() const
{
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    if (isNaN(carrierFrequency.get()))
        return m(par("maxInterferenceRange"));
    else
        return computeMaxRange(W(par("maxTransmissionPower")), mW(FWMath::dBm2mW(par("minInterferencePower"))));
}

const simtime_t RadioChannel::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t RadioChannel::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
}

bool RadioChannel::isInCommunicationRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxCommunicationRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxCommunicationRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxCommunicationRange.get());
}

bool RadioChannel::isInInterferenceRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxInterferenceRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxInterferenceRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxInterferenceRange.get());
}

bool RadioChannel::isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalListening *listening) const
{
    if (!isInInterferenceRange(transmission, listening->getStartPosition(), listening->getEndPosition()))
        return false;
    else
    {
        const IRadio *radio = listening->getReceiver();
        const IRadioSignalArrival *arrival = getArrival(radio, transmission);
        return arrival->getEndTime() >= listening->getStartTime() + minInterferenceTime && arrival->getStartTime() <= listening->getEndTime() - minInterferenceTime;
    }
}

bool RadioChannel::isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const
{
    const IRadioSignalArrival *arrival = getArrival(reception->getReceiver(), transmission);
    if (!isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition()))
        return false;
    else
        return arrival->getEndTime() > reception->getStartTime() + minInterferenceTime && arrival->getStartTime() < reception->getEndTime() - minInterferenceTime;
}

void RadioChannel::removeNonInterferingTransmissions()
{
    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;
    double minZ = DBL_MAX;
    double maxZ = DBL_MIN;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *radio = *it;
        IMobility *mobility = radio->getAntenna()->getMobility();
        Coord position = mobility->getCurrentPosition();
        if (position.x < minX)
            minX = position.x;
        if (position.x > maxX)
            maxX = position.x;
        if (position.y < minY)
            minY = position.y;
        if (position.y > maxY)
            maxY = position.y;
        if (position.z < minZ)
            minZ = position.z;
        if (position.z > maxZ)
            maxZ = position.z;
    }
    double distanceMax = Coord(minX, minY, minZ).distance(Coord(maxX, maxY, maxZ));
    double maxPropagationTime = distanceMax / propagation->getPropagationSpeed().get();
    simtime_t minInterferingTransmissionEndTime = simTime() - maxPropagationTime - maxTransmissionDuration;
    for (std::vector<const IRadioSignalTransmission *>::iterator it = transmissions.begin(); it != transmissions.end();)
    {
        const IRadioSignalTransmission *transmission = *it;
        if (transmission->getEndTime() < minInterferingTransmissionEndTime) {
            EV << "Removing non-interfering " << transmission << " from " << this << endl;
            const IRadioSignalTransmission *transmission = *it;
            transmissions.erase(it);
            // TODO: revive after fingerprint kuldges are removed from radio
            // delete transmission;
            const std::vector<const IRadioSignalArrival *> &transmissionArrivals = *(arrivals.begin() + (it - transmissions.begin()));
            for (std::vector<const IRadioSignalArrival *>::const_iterator jt = transmissionArrivals.begin(); jt != transmissionArrivals.end(); jt++)
                delete *jt;
            arrivals.erase(arrivals.begin() + (it - transmissions.begin()));
        }
        else
            it++;
    }
}

const IRadioSignalReception *RadioChannel::computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    return attenuation->computeReception(radio, transmission);
}

const std::vector<const IRadioSignalReception *> *RadioChannel::computeInterferingReceptions(const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const IRadio *radio = listening->getReceiver();
    std::vector<const IRadioSignalReception *> *interferingReceptions = new std::vector<const IRadioSignalReception *>();
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const IRadioSignalTransmission *interferingTransmission = *it;
        if (interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, listening))
            interferingReceptions->push_back(computeReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const std::vector<const IRadioSignalReception *> *RadioChannel::computeInterferingReceptions(const IRadioSignalReception *reception, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const IRadio *radio = reception->getReceiver();
    const IRadioSignalTransmission *transmission = reception->getTransmission();
    std::vector<const IRadioSignalReception *> *interferingReceptions = new std::vector<const IRadioSignalReception *>();
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const IRadioSignalTransmission *interferingTransmission = *it;
        if (transmission != interferingTransmission && isInterferingTransmission(interferingTransmission, reception))
            interferingReceptions->push_back(computeReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const IRadioSignalReceptionDecision *RadioChannel::computeReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const IRadioSignalReception *reception = computeReception(radio, transmission);
    const std::vector<const IRadioSignalReception *> *interferingReceptions = computeInterferingReceptions(reception, transmissions);
    const IRadioSignalNoise *noise = backgroundNoise ? backgroundNoise->computeNoise(reception) : NULL;
    const IRadioSignalReceptionDecision *decision = radio->getReceiver()->computeReceptionDecision(listening, reception, interferingReceptions, noise);
    delete noise;
    for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
        delete *it;
    delete interferingReceptions;
    return decision;
}

const IRadioSignalListeningDecision *RadioChannel::computeListeningDecision(const IRadio *radio, const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const std::vector<const IRadioSignalReception *> *interferingReceptions = computeInterferingReceptions(listening, transmissions);
    const IRadioSignalNoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const IRadioSignalListeningDecision *decision = radio->getReceiver()->computeListeningDecision(listening, interferingReceptions, noise);
    delete noise;
    for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
        delete *it;
    delete interferingReceptions;
    return decision;
}

void RadioChannel::transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission)
{
    transmissions.push_back(transmission);
    // TODO: delme
    std::vector<const IRadioSignalArrival *> transmissionArrivals;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *receiverRadio = *it;
        const IRadioSignalArrival *transmissionArrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
        transmissionArrivals.push_back(transmissionArrival);
    }
    arrivals.push_back(transmissionArrivals);
    removeNonInterferingTransmissions();
}

void RadioChannel::sendToChannel(IRadio *radio, const IRadioFrame *frame)
{
    const Radio *transmitterRadio = check_and_cast<Radio *>(radio);
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    EV_DEBUG << "Sending " << frame << " with " << radioFrame->getBitLength() << " bits in " << radioFrame->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << this << "." << endl;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const Radio *receiverRadio = check_and_cast<const Radio *>(*it);
        if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
        {
            cGate *gate = receiverRadio->RadioBase::getRadioGate()->getPathStartGate();
            const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
            simtime_t propagationTime = arrival->getStartPropagationTime();
            EV_DEBUG << "Sending " << frame
                     << " from " << radio << " at " << transmission->getStartPosition()
                     << " to " << *it << " at " << arrival->getStartPosition()
                     << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
            RadioFrame *frameCopy = new RadioFrame(radioFrame->getTransmission());
            frameCopy->encapsulate(radioFrame->getEncapsulatedPacket()->dup());
            const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, propagationTime, radioFrame->getDuration(), gate);
        }
    }
}

const IRadioSignalReceptionDecision *RadioChannel::receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    const IRadioSignalReceptionDecision *decision = computeReceptionDecision(radio, listening, transmission, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
    EV_DEBUG << "Receiving " << transmission << " from channel by " << radio << " arrives as " << decision->getReception() << " and results in " << decision << endl;
    return decision;
}

const IRadioSignalListeningDecision *RadioChannel::listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const
{
    const IRadioSignalListeningDecision *decision = computeListeningDecision(radio, listening, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
    EV_DEBUG << "Listening " << listening << " on channel by " << radio << " results in " << decision << endl;
    return decision;
}

bool RadioChannel::isPotentialReceiver(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    // TODO: KLUDGE: move
    const ScalarRadioSignalReceiver *scalarReceiver = dynamic_cast<const ScalarRadioSignalReceiver *>(radio->getReceiver());
    const ScalarRadioSignalTransmission *scalarTransmission = dynamic_cast<const ScalarRadioSignalTransmission *>(transmission);
    if (scalarReceiver && scalarTransmission && scalarTransmission->getCarrierFrequency() != scalarReceiver->getCarrierFrequency())
        return false;
    else {
        const IRadioSignalArrival *arrival = getArrival(radio, transmission);
        return isInCommunicationRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
}

bool RadioChannel::isReceptionAttempted(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    const IRadioSignalReception *reception = computeReception(radio, transmission);
    const std::vector<const IRadioSignalReception *> *interferingReceptions = computeInterferingReceptions(reception, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
    bool isReceptionAttempted = radio->getReceiver()->computeIsReceptionAttempted(reception, interferingReceptions);
    for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
        delete *it;
    delete interferingReceptions;
    EV_DEBUG << "Receiving " << transmission << " from channel by " << radio << " arrives as " << reception << " and results in reception is " << (isReceptionAttempted ? "attempted" : "ignored") << endl;
    return isReceptionAttempted;
}

const IRadioSignalArrival *RadioChannel::getArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    // KLUDGE: TODO: avoid doing a linear search here
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
    {
        if (*it == transmission)
        {
            const std::vector<const IRadioSignalArrival *> &transmissionArrivals = *(arrivals.begin() + (it - transmissions.begin()));
            return transmissionArrivals[radio->getId()];
        }
    }
    throw cRuntimeError("Arrival not found for transmission");
}

void RadioChannel::setArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival)
{
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
    {
        if (*it == transmission)
        {
            std::vector<const IRadioSignalArrival *> &transmissionArrivals = *(arrivals.begin() + (it - transmissions.begin()));
            transmissionArrivals[radio->getId()] = arrival;
        }
    }
}
