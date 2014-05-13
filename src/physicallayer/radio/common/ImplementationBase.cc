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

#include "ImplementationBase.h"
#include "ModuleAccess.h"
#include "IRadioChannel.h"

Define_Module(IsotropicRadioAntenna);
Define_Module(ConstantGainRadioAntenna);
Define_Module(ImmediateRadioSignalPropagation);
Define_Module(ConstantSpeedRadioSignalPropagation);

int RadioSignalTransmissionBase::nextId = 0;

void RadioSignalTransmissionBase::printToStream(std::ostream &stream) const
{
    stream << "transmission, id = " << id << ", transmitter id = " << transmitter->getId();
}

void RadioSignalListeningBase::printToStream(std::ostream &stream) const
{
    stream << "listening";
}

void RadioSignalReceptionBase::printToStream(std::ostream &stream) const
{
    stream << "reception, transmission id = " << transmission->getId() << ", receiver id = " << receiver->getId();
}

void RadioAntennaBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        mobility = check_and_cast<IMobility *>(getContainingNode(this)->getSubmodule("mobility"));
    }
}

void ConstantGainRadioAntenna::initialize(int stage)
{
    RadioAntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        gain = FWMath::dB2fraction(par("gain"));
    }
}

void RadioSignalFreeSpaceAttenuationBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        alpha = par("alpha");
    }
}

double RadioSignalFreeSpaceAttenuationBase::computePathLoss(const IRadioSignalTransmission *transmission, simtime_t receptionStartTime, simtime_t receptionEndTime, Coord receptionStartPosition, Coord receptionEndPosition, Hz carrierFrequency) const
{
    // factor = (waveLength / distance) ^ alpha / (16 * pi ^ 2)
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    m waveLength = transmission->getPropagationSpeed() / carrierFrequency;
    // this check allows to get the same result from the GPU and the CPU when the alpha is exactly 2
    double ratio = (waveLength / distance).get();
    double raisedRatio = alpha == 2.0 ? ratio * ratio : pow(ratio, alpha);
    double factor = distance.get() == 0 ? 1.0 : raisedRatio / (16.0 * M_PI * M_PI);
    EV_DEBUG << "Computing path loss with frequency = " << carrierFrequency << ", distance = " << distance << " results in factor = " << factor << endl;
    return factor;
}

void RadioSignalListeningDecision::printToStream(std::ostream &stream) const
{
    stream << "listening decision, " << (isListeningPossible_ ? "possible" : "impossible");
}

void RadioSignalReceptionDecision::printToStream(std::ostream &stream) const
{
    stream << "reception decision, " << (isReceptionPossible_ ? "possible" : "impossible");
    stream << ", " << (isReceptionSuccessful_ ? "successful" : "unsuccessful");
    stream << ", snir = " << snir;
}

bool RadioSignalReceiverBase::computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const
{
    return true;
}

bool RadioSignalReceiverBase::computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const
{
    if (!computeIsReceptionPossible(reception))
        return false;
    else if (simTime() == reception->getStartTime())
        // TODO: isn't there a better way for this optimization? see also in RadioChannel::isReceptionAttempted
        return !reception->getReceiver()->getReceptionInProgress();
    else
    {
        const IRadio *radio = reception->getReceiver();
        const IRadioChannel *channel = radio->getChannel();
        for (std::vector<const IRadioSignalReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++)
        {
            const IRadioSignalReception *interferingReception = *it;
            bool isPrecedingReception = interferingReception->getStartTime() < reception->getStartTime() ||
                                       (interferingReception->getStartTime() == reception->getStartTime() &&
                                        interferingReception->getTransmission()->getId() < reception->getTransmission()->getId());
            if (isPrecedingReception)
            {
                const IRadioSignalTransmission *interferingTransmission = interferingReception->getTransmission();
                if (interferingReception->getStartTime() <= simTime())
                {
                    if (radio->getReceptionInProgress() == interferingTransmission)
                        return false;
                }
                else if (channel->isReceptionAttempted(radio, interferingTransmission))
                    return false;
            }
        }
        return true;
    }
}

void SNIRRadioSignalReceiverBase::initialize(int stage)
{
    RadioSignalReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        snirThreshold = FWMath::dB2fraction(par("snirThreshold"));
    }
}

const IRadioSignalReceptionDecision *SNIRRadioSignalReceiverBase::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const IRadioSignalNoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    double snirMin = computeSNIRMin(reception, noise);
    bool isReceptionPossible = computeIsReceptionPossible(reception);
    bool isReceptionSuccessful = isReceptionPossible && snirMin > snirThreshold;
    delete noise;
    return new RadioSignalReceptionDecision(reception, isReceptionPossible, isReceptionSuccessful, snirMin);
}

const IRadioSignalArrival *ImmediateRadioSignalPropagation::computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const
{
    const Coord position = mobility->getCurrentPosition();
    return new RadioSignalArrival(0.0, 0.0, transmission->getStartTime(), transmission->getEndTime(), position, position);
}

void ConstantSpeedRadioSignalPropagation::finish()
{
    EV_INFO << "Radio signal arrival computation count = " << arrivalComputationCount << endl;
    recordScalar("Radio signal arrival computation count", arrivalComputationCount);
}

const Coord ConstantSpeedRadioSignalPropagation::computeArrivalPosition(const simtime_t time, const Coord position, IMobility *mobility) const
{
    switch (mobilityApproximationCount)
    {
        case 0:
            return mobility->getCurrentPosition();
        case 1:
            return mobility->getPosition(time);
        case 2:
        {
            // NOTE: repeat once again to approximate the movement during propagation
            double distance = position.distance(mobility->getPosition(time));
            simtime_t propagationTime = distance / propagationSpeed.get();
            return mobility->getPosition(time + propagationTime);
        }
        default:
            throw cRuntimeError("Unknown mobility approximation");
    }
}

const IRadioSignalArrival *ConstantSpeedRadioSignalPropagation::computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord startPosition = transmission->getStartPosition();
    const Coord endPosition = transmission->getEndPosition();
    const Coord startArrivalPosition = computeArrivalPosition(startTime, startPosition, mobility);
    const Coord endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / propagationSpeed.get();
    const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const simtime_t endArrivalTime = endTime + endPropagationTime;
    return new RadioSignalArrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition);
}
