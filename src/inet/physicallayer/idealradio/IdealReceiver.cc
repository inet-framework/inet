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

#include "inet/physicallayer/idealradio/IdealReceiver.h"
#include "inet/physicallayer/idealradio/IdealListening.h"
#include "inet/physicallayer/idealradio/IdealReception.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"

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

std::ostream& IdealReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "IdealReceiver";
    if (level >= PRINT_LEVEL_TRACE)
        stream << (ignoreInterference ? ", ignoring interference" : ", considering interference");
    return stream;
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

bool IdealReceiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const
{
    const IdealReception::Power power = check_and_cast<const IdealReception *>(reception)->getPower();
    if (power == IdealReception::POWER_RECEIVABLE) {
        if (ignoreInterference)
            return true;
        else {
            const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
            for (auto interferingReception : *interferingReceptions) {

                IdealReception::Power interferingPower = check_and_cast<const IdealReception *>(interferingReception)->getPower();
                if (interferingPower == IdealReception::POWER_RECEIVABLE || interferingPower == IdealReception::POWER_INTERFERING)
                    return false;
            }
            return true;
        }
    }
    else
        return false;
}

const IListening *IdealReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new IdealListening(radio, startTime, endTime, startPosition, endPosition);
}

const IListeningDecision *IdealReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto interferingReception : *interferingReceptions) {

        IdealReception::Power interferingPower = check_and_cast<const IdealReception *>(interferingReception)->getPower();
        if (interferingPower != IdealReception::POWER_UNDETECTABLE)
            return new ListeningDecision(listening, true);
    }
    return new ListeningDecision(listening, false);
}

const IReceptionDecision *IdealReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const
{
    ReceptionIndication *indication = new ReceptionIndication();
    bool isReceptionSuccessful = computeIsReceptionSuccessful(listening, reception, interference, snir);
    double errorRate = isReceptionSuccessful ? 0 : 1;
    indication->setSymbolErrorRate(errorRate);
    indication->setBitErrorRate(errorRate);
    indication->setPacketErrorRate(errorRate);
    return new ReceptionDecision(reception, indication, true, true, isReceptionSuccessful);
}

} // namespace physicallayer

} // namespace inet

