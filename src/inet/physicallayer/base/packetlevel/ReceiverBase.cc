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

#include "inet/physicallayer/base/packetlevel/ReceiverBase.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionResult.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const
{
    if (!computeIsReceptionPossible(listening, reception, part))
        return false;
    else if (simTime() == reception->getStartTime(part)) {
        // TODO: isn't there a better way for this optimization? see also in RadioMedium::isReceptionAttempted
        auto transmission = reception->getReceiver()->getReceptionInProgress();
        return transmission == nullptr || transmission == reception->getTransmission();
    }
    else {
        // determining whether the reception is attempted or not for the future
        auto radio = reception->getReceiver();
        auto radioMedium = radio->getMedium();
        auto interferingReceptions = interference->getInterferingReceptions();
        for (auto interferingReception : *interferingReceptions) {
            auto isPrecedingReception = interferingReception->getStartTime() < reception->getStartTime() ||
                (interferingReception->getStartTime() == reception->getStartTime() &&
                 interferingReception->getTransmission()->getId() < reception->getTransmission()->getId());
            if (isPrecedingReception) {
                auto interferingTransmission = interferingReception->getTransmission();
                if (interferingReception->getStartTime() <= simTime()) {
                    if (radio->getReceptionInProgress() == interferingTransmission)
                        return false;
                }
                else if (radioMedium->isReceptionAttempted(radio, interferingTransmission, part))
                    return false;
            }
        }
        return true;
    }
}

const ReceptionIndication *ReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    return createReceptionIndication();
}

ReceptionIndication *ReceiverBase::createReceptionIndication() const
{
    return new ReceptionIndication();
}

const IReceptionDecision *ReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const
{
    auto isReceptionPossible = computeIsReceptionPossible(listening, reception, part);
    auto isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, part, interference);
    auto isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    return new ReceptionDecision(reception, part, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

const IReceptionResult *ReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const
{
    auto radio = reception->getReceiver();
    auto radioMedium = radio->getMedium();
    auto transmission = reception->getTransmission();
    auto indication = computeReceptionIndication(snir);
    // TODO: add all cached decisions?
    auto decisions = new std::vector<const IReceptionDecision *>();
    decisions->push_back(radioMedium->getReceptionDecision(radio, listening, transmission, IRadioSignal::SIGNAL_PART_WHOLE));
    return new ReceptionResult(reception, decisions, indication);
}

} // namespace physicallayer

} // namespace inet

