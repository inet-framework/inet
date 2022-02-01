//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

Ieee80211TransmissionBase::Ieee80211TransmissionBase(const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    mode(mode),
    channel(channel)
{
}

std::ostream& Ieee80211TransmissionBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(mode, printFieldToString(mode, level + 1, evFlags))
               << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return stream;
}

} // namespace physicallayer

} // namespace inet

