//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"

namespace inet {

namespace physicallayer {

Ieee80211Channel::Ieee80211Channel(const IIeee80211Band *band, int channelNumber, Hz width) :
    Ieee80211Channel(band, channelNumber)
{
    allocationWidth = width;
    // IEEE Std 802.11-2024, Annex E/Table E-4 classes115--120,128,129.
    // This is an explicitly bounded model allocation, not regulatory permission.
    int primary = getPrimaryChannel();
    if (primary < 36 || primary > 64 || primary % 4 != 0
            || (width != MHz(20) && width != MHz(40) && width != MHz(80) && width != MHz(160))
            || band->getCenterFrequency(channelNumber) != MHz(5000 + 5 * primary))
        throw cRuntimeError("Unsupported contiguous IEEE 802.11 allocation");
}

int Ieee80211Channel::getCenterChannel(Hz width) const
{
    if (!hasAllocation() || width > allocationWidth)
        throw cRuntimeError("IEEE 802.11 width is outside the configured allocation");
    int primary = getPrimaryChannel();
    if (width == MHz(20))
        return primary;
    if (width == MHz(40))
        return 38 + ((primary - 36) / 8) * 8;
    if (width == MHz(80))
        return primary < 52 ? 42 : 58;
    if (width == MHz(160))
        return 50;
    throw cRuntimeError("Unsupported IEEE 802.11 channel width");
}

Hz Ieee80211Channel::getCenterFrequency(Hz width) const
{
    return MHz(5000 + 5 * getCenterChannel(width));
}

bool Ieee80211Channel::containsPrimaryAllocation(Hz center, Hz width) const
{
    return hasAllocation() && width <= allocationWidth
        && (width == MHz(20) || width == MHz(40) || width == MHz(80) || width == MHz(160))
        && center == getCenterFrequency(width);
}

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

