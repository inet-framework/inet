//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/common/base/packetlevel/SnirReceiverBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

void SnirReceiverBase::initialize(int stage)
{
    ReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        snirThreshold = math::dB2fraction(par("snirThreshold"));
        const char *snirThresholdModeString = par("snirThresholdMode");
        if (!strcmp("min", snirThresholdModeString))
            snirThresholdMode = SnirThresholdMode::STM_MIN;
        else if (!strcmp("mean", snirThresholdModeString))
            snirThresholdMode = SnirThresholdMode::STM_MEAN;
        else
            throw cRuntimeError("Unknown SNIR threshold mode: '%s'", snirThresholdModeString);
    }
}

std::ostream& SnirReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(snirThreshold);
    return stream;
}

bool SnirReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    if (snirThresholdMode == SnirThresholdMode::STM_MIN) {
        double minSnir = snir->getMin();
        ASSERT(0.0 <= minSnir);
        return minSnir > snirThreshold;
    }
    else if (snirThresholdMode == SnirThresholdMode::STM_MEAN) {
        double meanSnir = snir->getMean();
        ASSERT(0.0 <= meanSnir);
        return meanSnir > snirThreshold;
    }
    else
        throw cRuntimeError("Unknown SNIR threshold mode: '%s'", snirThresholdMode);
}

} // namespace physicallayer

} // namespace inet

