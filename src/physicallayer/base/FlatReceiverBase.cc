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

#include "FlatReceiverBase.h"
#include "FlatTransmissionBase.h"
#include "FlatReceptionBase.h"
#include "FlatNoiseBase.h"
#include "Modulation.h"
#include "BandListening.h"
#include "ListeningDecision.h"
#include "ReceptionDecision.h"

namespace inet {

namespace physicallayer {

void FlatReceiverBase::initialize(int stage)
{
    SNIRReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        const char *modulationName = par("modulation");
        if (strcmp(modulationName, "NULL") == 0)
            modulation = new NullModulation();
        else if (strcmp(modulationName, "BPSK") == 0)
            modulation = new BPSKModulation();
        else if (strcmp(modulationName, "16-QAM") == 0)
            modulation = new QAM16Modulation();
        else if (strcmp(modulationName, "256-QAM") == 0)
            modulation = new QAM256Modulation();
        else
            throw cRuntimeError(this, "Unknown modulation '%s'", modulationName);
    }
}

void FlatReceiverBase::printToStream(std::ostream& stream) const
{
    stream << "Flat receiver, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " << sensitivity;
}

const IListening *FlatReceiverBase::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

bool FlatReceiverBase::computeIsReceptionPossible(const ITransmission *transmission) const
{
    // TODO: check if modulation matches?
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    return carrierFrequency == flatTransmission->getCarrierFrequency() && bandwidth == flatTransmission->getBandwidth();
}

// TODO: this is not purely functional, see interface comment
bool FlatReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
    return bandListening->getCarrierFrequency() == flatReception->getCarrierFrequency() && bandListening->getBandwidth() == flatReception->getBandwidth() &&
           flatReception->computeMinPower(reception->getStartTime(), reception->getEndTime()) >= sensitivity;
}

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    const INoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    const FlatNoiseBase *flatNoise = check_and_cast<const FlatNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    delete noise;
    return new ListeningDecision(listening, maxPower >= energyDetection);
}

// TODO: this is not purely functional, see interface comment
bool FlatReceiverBase::computeHasBitError(const IListening *listening, double minSNIR, int bitLength, double bitrate) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    double ber = modulation->calculateBER(minSNIR, bandwidth.get(), bitrate);
    if (ber == 0.0)
        return false;
    else {
        double pErrorless = pow(1.0 - ber, bitLength);
        return dblrand() > pErrorless;
    }
}

bool FlatReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, const RadioReceptionIndication *indication) const
{
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
    return SNIRReceiverBase::computeIsReceptionSuccessful(listening, reception, indication) &&
           !computeHasBitError(listening, indication->getMinSNIR(), flatTransmission->getPayloadBitLength(), flatTransmission->getBitrate().get());
}

const IReceptionDecision *FlatReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
    if (bandListening->getCarrierFrequency() == flatReception->getCarrierFrequency() && bandListening->getBandwidth() == flatReception->getBandwidth())
        return SNIRReceiverBase::computeReceptionDecision(listening, reception, interferingReceptions, backgroundNoise);
    else
        return new ReceptionDecision(reception, new RadioReceptionIndication(), false, false, false);
}

} // namespace physicallayer

} // namespace inet

