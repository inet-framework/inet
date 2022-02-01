//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"

namespace inet {
namespace physicallayer {

ListeningDecision::ListeningDecision(const IListening *listening, bool isListeningPossible_) :
    listening(listening),
    isListeningPossible_(isListeningPossible_)
{
}

std::ostream& ListeningDecision::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ListeningDecision";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << (isListeningPossible_ ? ", \x1b[1mpossible\x1b[0m" : ", \x1b[1mimpossible\x1b[0m");
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(listening, printFieldToString(listening, level + 1, evFlags));
    return stream;
}

} // namespace physicallayer
} // namespace inet

