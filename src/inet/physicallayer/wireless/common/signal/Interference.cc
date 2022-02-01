//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/signal/Interference.h"

namespace inet {
namespace physicallayer {

Interference::Interference(const INoise *backgroundNoise, const std::vector<const IReception *> *interferingReceptions) :
    backgroundNoise(backgroundNoise),
    interferingReceptions(interferingReceptions)
{
}

Interference::~Interference()
{
    delete backgroundNoise;
    delete interferingReceptions;
}

std::ostream& Interference::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Interference";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(backgroundNoise, printFieldToString(backgroundNoise, level + 1, evFlags))
               << EV_FIELD(interferingReceptions, interferingReceptions);
    return stream;
}

} // namespace physicallayer
} // namespace inet

