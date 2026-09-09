//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"

namespace inet {

namespace physicallayer {

Ieee80211Channel::Ieee80211Channel(const IIeee80211Band *band, int channelNumber) :
    Ieee80211Channel(band, channelNumber, IEEE80211_SECONDARY_CHANNEL_NONE)
{
}

Ieee80211Channel::Ieee80211Channel(const IIeee80211Band *band, int channelNumber, Ieee80211SecondaryChannelOffset secondaryChannelOffset) :
    band(band),
    channelNumber(channelNumber),
    secondaryChannelOffset(secondaryChannelOffset)
{
    // IEEE Std 802.11-2024, Table 9-134 reserves wire value 2; 19.2.3 and
    // 19.3.15.4 place the secondary channel four channel numbers away.
    if (secondaryChannelOffset != IEEE80211_SECONDARY_CHANNEL_NONE &&
            secondaryChannelOffset != IEEE80211_SECONDARY_CHANNEL_ABOVE &&
            secondaryChannelOffset != IEEE80211_SECONDARY_CHANNEL_BELOW)
        throw cRuntimeError("Invalid IEEE 802.11 secondary channel offset: %d", (int)secondaryChannelOffset);
    if (secondaryChannelOffset != IEEE80211_SECONDARY_CHANNEL_NONE) {
        if (band == nullptr)
            throw cRuntimeError("Cannot resolve an IEEE 802.11 secondary channel without a band");
        (void)getCenterFrequency();
        (void)getSecondaryCenterFrequency();
    }
}

Ieee80211SecondaryChannelOffset Ieee80211Channel::parseSecondaryChannelOffset(const char *text)
{
    if (!strcmp(text, "none"))
        return IEEE80211_SECONDARY_CHANNEL_NONE;
    if (!strcmp(text, "above"))
        return IEEE80211_SECONDARY_CHANNEL_ABOVE;
    if (!strcmp(text, "below"))
        return IEEE80211_SECONDARY_CHANNEL_BELOW;
    throw cRuntimeError("Unknown IEEE 802.11 secondary channel offset '%s'", text);
}

const char *Ieee80211Channel::getSecondaryChannelOffsetName(Ieee80211SecondaryChannelOffset offset)
{
    switch (offset) {
        case IEEE80211_SECONDARY_CHANNEL_NONE:
            return "none";
        case IEEE80211_SECONDARY_CHANNEL_ABOVE:
            return "above";
        case IEEE80211_SECONDARY_CHANNEL_BELOW:
            return "below";
        default:
            throw cRuntimeError("Unknown IEEE 802.11 secondary channel offset: %d", (int)offset);
    }
}

int Ieee80211Channel::getSecondaryChannelNumber() const
{
    switch (secondaryChannelOffset) {
        case IEEE80211_SECONDARY_CHANNEL_NONE:
            return channelNumber;
        case IEEE80211_SECONDARY_CHANNEL_ABOVE:
            return channelNumber + 4;
        case IEEE80211_SECONDARY_CHANNEL_BELOW:
            return channelNumber - 4;
        default:
            throw cRuntimeError("Unknown secondary channel offset: %d", secondaryChannelOffset);
    }
}

Hz Ieee80211Channel::getSecondaryCenterFrequency() const
{
    int direction = secondaryChannelOffset == IEEE80211_SECONDARY_CHANNEL_ABOVE ? 1 :
            secondaryChannelOffset == IEEE80211_SECONDARY_CHANNEL_BELOW ? -1 : 0;
    if (direction == 0)
        throw cRuntimeError("IEEE 802.11 channel has no secondary channel");
    int secondaryChannelNumber = channelNumber + 4 * direction;
    Hz secondaryCenterFrequency = band->getCenterFrequency(secondaryChannelNumber);
    if (secondaryCenterFrequency != getCenterFrequency() + MHz(20 * direction))
        throw cRuntimeError("IEEE 802.11 secondary channel is not 20 MHz from the primary channel");
    return secondaryCenterFrequency;
}

Hz Ieee80211Channel::getBondedCenterFrequency() const
{
    switch (secondaryChannelOffset) {
        case IEEE80211_SECONDARY_CHANNEL_NONE:
            return getCenterFrequency();
        case IEEE80211_SECONDARY_CHANNEL_ABOVE:
            return getCenterFrequency() + MHz(10);
        case IEEE80211_SECONDARY_CHANNEL_BELOW:
            return getCenterFrequency() - MHz(10);
        default:
            throw cRuntimeError("Invalid IEEE 802.11 secondary channel offset: %d", (int)secondaryChannelOffset);
    }
}

std::ostream& Ieee80211Channel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211Channel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(channelNumber)
               << EV_FIELD(secondaryChannelOffset);
    return stream;
}

} // namespace physicallayer

} // namespace inet

