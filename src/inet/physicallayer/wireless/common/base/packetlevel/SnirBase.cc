//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

SnirBase::SnirBase(const IReception *reception, const INoise *noise) :
    reception(reception),
    noise(noise)
{
}

std::ostream& SnirBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(reception, printFieldToString(reception, level + 1, evFlags))
               << EV_FIELD(noise, printFieldToString(noise, level + 1, evFlags));
    return stream;
}

} // namespace physicallayer

} // namespace inet

