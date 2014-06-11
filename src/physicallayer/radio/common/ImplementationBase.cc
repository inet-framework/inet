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
#include "GenericImplementation.h"
#include "ModuleAccess.h"

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
        mobility = check_and_cast<IMobility *>(getContainingNode(this)->getSubmodule("mobility"));
}

EulerAngles RadioSignalAttenuationBase::computeTransmissionDirection(const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const
{
    const Coord transmissionStartPosition = transmission->getStartPosition();
    const Coord arrivalStartPosition = arrival->getStartPosition();
    Coord transmissionStartDirection = arrivalStartPosition - transmissionStartPosition;
    double z = transmissionStartDirection.z;
    transmissionStartDirection.z = 0;
    double heading = atan2(transmissionStartDirection.y, transmissionStartDirection.x);
    double elevation = atan2(z, transmissionStartDirection.length());
    return EulerAngles(heading, elevation, 0);
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
        snirThreshold = FWMath::dB2fraction(par("snirThreshold"));
}

bool SNIRRadioSignalReceiverBase::areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const
{
    return carrierFrequency1 + bandwidth1 / 2 >= carrierFrequency2 - bandwidth2 / 2 &&
           carrierFrequency1 - bandwidth1 / 2 <= carrierFrequency2 + bandwidth2 / 2;
}

const RadioReceptionIndication *SNIRRadioSignalReceiverBase::computeReceptionIndication(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const IRadioSignalNoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    double minSNIR = computeMinSNIR(reception, noise);
    delete noise;
    RadioReceptionIndication *indication = new RadioReceptionIndication();
    indication->setMinSNIR(minSNIR);
    return indication;
}

bool SNIRRadioSignalReceiverBase::computeIsReceptionSuccessful(const IRadioSignalReception *reception, const RadioReceptionIndication *indication) const
{
    return indication->getMinSNIR() > snirThreshold;
}

const IRadioSignalReceptionDecision *SNIRRadioSignalReceiverBase::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    bool isReceptionPossible = computeIsReceptionPossible(reception);
    bool isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(reception, interferingReceptions);
    const RadioReceptionIndication *indication = isReceptionAttempted ? computeReceptionIndication(listening, reception, interferingReceptions, backgroundNoise) : NULL;
    bool isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(reception, indication);
    return new RadioSignalReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

RadioSignalPropagationBase::RadioSignalPropagationBase() :
    propagationSpeed(mps(sNaN)),
    arrivalComputationCount(0)
{}

void RadioSignalPropagationBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        propagationSpeed = mps(par("propagationSpeed"));
}

void RadioSignalPropagationBase::finish()
{
    EV_INFO << "Radio signal arrival computation count = " << arrivalComputationCount << endl;
    recordScalar("Radio signal arrival computation count", arrivalComputationCount);
}
