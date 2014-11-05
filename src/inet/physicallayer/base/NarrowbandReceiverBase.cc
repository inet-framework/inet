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

#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/base/NarrowbandReceiverBase.h"
#include "inet/physicallayer/base/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/base/NarrowbandReceptionBase.h"
#include "inet/physicallayer/base/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/Modulation.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/common/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

NarrowbandReceiverBase::NarrowbandReceiverBase() :
    SNIRReceiverBase(),
    modulation(NULL),
    errorModel(NULL),
    energyDetection(W(sNaN)),
    sensitivity(W(sNaN)),
    carrierFrequency(Hz(sNaN)),
    bandwidth(Hz(sNaN))
{
}

NarrowbandReceiverBase::~NarrowbandReceiverBase()
{
    delete errorModel;
    delete modulation;
}

void NarrowbandReceiverBase::initialize(int stage)
{
    SNIRReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        const char *modulationName = par("modulation");
        if (strcmp(modulationName, "BPSK") == 0)
            modulation = new BPSKModulation();
        else if (strcmp(modulationName, "16-QAM") == 0)
            modulation = new QAM16Modulation();
        else if (strcmp(modulationName, "256-QAM") == 0)
            modulation = new QAM256Modulation();
        else
            throw cRuntimeError(this, "Unknown modulation '%s'", modulationName);
    }
}

void NarrowbandReceiverBase::printToStream(std::ostream& stream) const
{
    stream << "modulation = { " << modulation << " }, "
           << "errorModel = { " << errorModel << " }, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " << sensitivity << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth << ", ";
    SNIRReceiverBase::printToStream(stream);
}

const IListening *NarrowbandReceiverBase::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

bool NarrowbandReceiverBase::computeIsReceptionPossible(const ITransmission *transmission) const
{
    // TODO: check if modulation matches?
    const NarrowbandTransmissionBase *flatTransmission = check_and_cast<const NarrowbandTransmissionBase *>(transmission);
    return carrierFrequency == flatTransmission->getCarrierFrequency() && bandwidth == flatTransmission->getBandwidth();
}

// TODO: this is not purely functional, see interface comment
bool NarrowbandReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *flatReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCarrierFrequency() != flatReception->getCarrierFrequency() || bandListening->getBandwidth() != flatReception->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        W minReceptionPower = flatReception->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

const IListeningDecision *NarrowbandReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *flatNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing listening possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

bool NarrowbandReceiverBase::computeIsReceptionSuccessful(const ISNIR *snir) const
{
    if (!SNIRReceiverBase::computeIsReceptionSuccessful(snir))
        return false;
    else if (!errorModel)
        return true;
    else {
        double packetErrorRate = errorModel->computePacketErrorRate(snir);
        if (packetErrorRate == 0.0)
            return true;
        else if (packetErrorRate == 1.0)
            return false;
        else
            return dblrand() > packetErrorRate;
    }
}

const RadioReceptionIndication *NarrowbandReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    RadioReceptionIndication *indication = const_cast<RadioReceptionIndication *>(SNIRReceiverBase::computeReceptionIndication(snir));
    if (errorModel) {
        indication->setPacketErrorRate(errorModel->computePacketErrorRate(snir));
        indication->setBitErrorRate(errorModel->computeBitErrorRate(snir));
        indication->setSymbolErrorRate(errorModel->computeSymbolErrorRate(snir));
    }
    return indication;
}

const IReceptionDecision *NarrowbandReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *flatReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCarrierFrequency() == flatReception->getCarrierFrequency() && bandListening->getBandwidth() == flatReception->getBandwidth())
        return SNIRReceiverBase::computeReceptionDecision(listening, reception, interference);
    else
        return new ReceptionDecision(reception, new RadioReceptionIndication(), false, false, false);
}

} // namespace physicallayer

} // namespace inet

