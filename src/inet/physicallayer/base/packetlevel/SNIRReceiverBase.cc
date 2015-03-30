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

#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/base/packetlevel/SNIRReceiverBase.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

SNIRReceiverBase::SNIRReceiverBase() :
    ReceiverBase(),
    snirThreshold(sNaN)
{
}

void SNIRReceiverBase::initialize(int stage)
{
    ReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        snirThreshold = math::dB2fraction(par("snirThreshold"));
}

void SNIRReceiverBase::printToStream(std::ostream& stream) const
{
    stream << "snirThreshold = " << snirThreshold;
}

const ReceptionIndication *SNIRReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    ReceptionIndication *indication = new ReceptionIndication();
    indication->setMinSNIR(snir->getMin());
    return indication;
}

bool SNIRReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const ITransmission *transmission = reception->getTransmission();
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const ISNIR *snir = medium->getSNIR(receiver, transmission);
    return snir->getMin() > snirThreshold;
}

const IReceptionDecision *SNIRReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const ITransmission *transmission = reception->getTransmission();
    const ISNIR *snir = medium->getSNIR(receiver, transmission);
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception);
    bool isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, interference);
    bool isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(listening, reception, interference);
    const ReceptionIndication *indication = isReceptionAttempted ? computeReceptionIndication(snir) : nullptr;
    return new ReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

} // namespace physicallayer

} // namespace inet

