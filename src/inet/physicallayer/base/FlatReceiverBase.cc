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

#include "inet/physicallayer/base/FlatReceiverBase.h"
#include "inet/physicallayer/base/FlatTransmissionBase.h"
#include "inet/physicallayer/base/FlatReceptionBase.h"
#include "inet/physicallayer/base/FlatNoiseBase.h"
#include "inet/physicallayer/common/Modulation.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/common/ReceptionDecision.h"
#include "inet/physicallayer/common/FlatErrorModel.h"

namespace inet {

namespace physicallayer {

FlatReceiverBase::FlatReceiverBase() :
    SNIRReceiverBase(),
    modulator(NULL),
    errorModel(NULL),
    energyDetection(W(sNaN)),
    sensitivity(W(sNaN)),
    carrierFrequency(Hz(sNaN)),
    bandwidth(Hz(sNaN))
{
}

FlatReceiverBase::~FlatReceiverBase()
{
    delete errorModel;
}

void FlatReceiverBase::initialize(int stage)
{
    SNIRReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        // TODO: modulator will be a module
    }
}

void FlatReceiverBase::printToStream(std::ostream& stream) const
{
    stream << "modulation = {" << "todo" << "}, "
           << "error model = {" << errorModel << "}, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " << sensitivity << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth << ", ";
    SNIRReceiverBase::printToStream(stream);
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

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const INoise *noise = computeNoise(listening, interference);
    const FlatNoiseBase *flatNoise = check_and_cast<const FlatNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    delete noise;
    return new ListeningDecision(listening, maxPower >= energyDetection);
}

bool FlatReceiverBase::computeIsReceptionSuccessful(const ISNIR *snir) const
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

const RadioReceptionIndication *FlatReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    RadioReceptionIndication *indication = const_cast<RadioReceptionIndication *>(SNIRReceiverBase::computeReceptionIndication(snir));
    if (errorModel) {
        indication->setPacketErrorRate(errorModel->computePacketErrorRate(snir));
        indication->setBitErrorRate(errorModel->computeBitErrorRate(snir));
        indication->setSymbolErrorRate(errorModel->computeSymbolErrorRate(snir));
    }
    return indication;
}

const IReceptionDecision *FlatReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
    if (bandListening->getCarrierFrequency() == flatReception->getCarrierFrequency() && bandListening->getBandwidth() == flatReception->getBandwidth())
        return SNIRReceiverBase::computeReceptionDecision(listening, reception, interference);
    else
        return new ReceptionDecision(reception, new RadioReceptionIndication(), false, false, false);
}

} // namespace physicallayer

} // namespace inet

