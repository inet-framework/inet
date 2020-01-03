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

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandReceptionBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

NarrowbandReceiverBase::NarrowbandReceiverBase() :
    SnirReceiverBase(),
    modulation(nullptr),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN))
{
}

void NarrowbandReceiverBase::initialize(int stage)
{
    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        modulation = ApskModulationBase::findModulation(par("modulation"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

std::ostream& NarrowbandReceiverBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", modulation = " << printObjectToString(modulation, level + 1)
               << ", centerFrequency = " << centerFrequency
               << ", bandwidth = " << bandwidth;
    return SnirReceiverBase::printToStream(stream, level);
}

const IListening *NarrowbandReceiverBase::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);
}

bool NarrowbandReceiverBase::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    // TODO: check if modulation matches?
    const NarrowbandTransmissionBase *narrowbandTransmission = check_and_cast<const NarrowbandTransmissionBase *>(transmission);
    return centerFrequency == narrowbandTransmission->getCenterFrequency() && bandwidth >= narrowbandTransmission->getBandwidth();
}

// TODO: this is not purely functional, see interface comment
bool NarrowbandReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *narrowbandReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCenterFrequency() != narrowbandReception->getCenterFrequency() || bandListening->getBandwidth() < narrowbandReception->getBandwidth()) {
        EV_DEBUG << "Computing whether reception is possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else
        return true;
}

const IReceptionDecision *NarrowbandReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *narrowbandReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCenterFrequency() == narrowbandReception->getCenterFrequency() && bandListening->getBandwidth() >= narrowbandReception->getBandwidth())
        return SnirReceiverBase::computeReceptionDecision(listening, reception, part, interference, snir);
    else
        return new ReceptionDecision(reception, part, false, false, false);
}

} // namespace physicallayer

} // namespace inet

