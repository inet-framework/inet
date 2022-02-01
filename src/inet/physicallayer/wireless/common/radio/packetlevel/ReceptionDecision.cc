//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {
namespace physicallayer {

ReceptionDecision::ReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful) :
    reception(reception),
    part(part),
    isReceptionPossible_(isReceptionPossible),
    isReceptionAttempted_(isReceptionAttempted),
    isReceptionSuccessful_(isReceptionSuccessful)
{
}

std::ostream& ReceptionDecision::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ReceptionDecision";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << (isReceptionPossible_ ? ", possible" : ", impossible")
               << (isReceptionAttempted_ ? ", attempted" : ", ignored")
               << (isReceptionSuccessful_ ? ", successful" : ", unsuccessful");
    return stream;
}

} // namespace physicallayer
} // namespace inet

