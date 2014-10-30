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

#include "inet/physicallayer/ideal/IdealReceiver.h"
#include "inet/physicallayer/ideal/IdealListening.h"
#include "inet/physicallayer/ideal/IdealReception.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/common/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

Define_Module(IdealReceiver);

IdealReceiver::IdealReceiver() :
    ignoreInterference(false)
{
}

void IdealReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ignoreInterference = par("ignoreInterference");
    }
}

bool IdealReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    const IdealReception::Power power = check_and_cast<const IdealReception *>(reception)->getPower();
    return power == IdealReception::POWER_RECEIVABLE;
}

bool IdealReceiver::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    if (ignoreInterference)
        return computeIsReceptionPossible(listening, reception);
    else
        return ReceiverBase::computeIsReceptionAttempted(listening, reception, interference);
}

void IdealReceiver::printToStream(std::ostream& stream) const
{
    stream << "IdealReceiver, "
           << (ignoreInterference ? "ignoring interference" : "considering interference");
}

const IListening *IdealReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new IdealListening(radio, startTime, endTime, startPosition, endPosition);
}

const IListeningDecision *IdealReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (std::vector<const IReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++) {
        const IReception *interferingReception = *it;
        IdealReception::Power interferingPower = check_and_cast<const IdealReception *>(interferingReception)->getPower();
        if (interferingPower != IdealReception::POWER_UNDETECTABLE)
            return new ListeningDecision(listening, true);
    }
    return new ListeningDecision(listening, false);
}

const IReceptionDecision *IdealReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IdealReception::Power power = check_and_cast<const IdealReception *>(reception)->getPower();
    RadioReceptionIndication *indication = new RadioReceptionIndication();
    if (power == IdealReception::POWER_RECEIVABLE) {
        if (ignoreInterference)
            return new ReceptionDecision(reception, indication, true, true, true);
        else {
            const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
            for (std::vector<const IReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++) {
                const IReception *interferingReception = *it;
                IdealReception::Power interferingPower = check_and_cast<const IdealReception *>(interferingReception)->getPower();
                if (interferingPower == IdealReception::POWER_RECEIVABLE || interferingPower == IdealReception::POWER_INTERFERING)
                    return new ReceptionDecision(reception, indication, true, true, false);
            }
            return new ReceptionDecision(reception, indication, true, true, true);
        }
    }
    else
        return new ReceptionDecision(reception, indication, false, false, false);
}

} // namespace physicallayer

} // namespace inet

