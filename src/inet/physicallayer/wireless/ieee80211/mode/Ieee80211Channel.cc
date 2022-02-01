//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"

namespace inet {

namespace physicallayer {

Ieee80211Channel::Ieee80211Channel(const IIeee80211Band *band, int channelNumber) :
    band(band),
    channelNumber(channelNumber)
{
}

std::ostream& Ieee80211Channel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211Channel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(channelNumber);
    return stream;
}

} // namespace physicallayer

} // namespace inet

