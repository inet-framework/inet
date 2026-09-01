//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"

namespace inet {

namespace physicallayer {

Ieee80211BandBase::Ieee80211BandBase(const char *name) :
    name(name)
{
}

int IIeee80211Band::getStandardChannelNumber(int channelIndex) const
{
    throw cRuntimeError("Band '%s' has no standards channel-number mapping for internal channel index %d", getName(), channelIndex);
}

int IIeee80211Band::getChannelIndex(int standardChannelNumber) const
{
    throw cRuntimeError("Band '%s' has no internal channel-index mapping for standards channel number %d", getName(), standardChannelNumber);
}

bool IIeee80211Band::isHt40OperationSupported(int primaryChannelIndex, int secondaryChannelOffset) const
{
    if (secondaryChannelOffset != 1 && secondaryChannelOffset != 3)
        return false;
    try {
        int primaryStandardChannel = getStandardChannelNumber(primaryChannelIndex);
        int secondaryStandardChannel = primaryStandardChannel + (secondaryChannelOffset == 1 ? 4 : -4);
        int secondaryChannelIndex = getChannelIndex(secondaryStandardChannel);
        Hz expectedOffset = secondaryChannelOffset == 1 ? MHz(20) : MHz(-20);
        return getCenterFrequency(secondaryChannelIndex) - getCenterFrequency(primaryChannelIndex) == expectedOffset;
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

Ieee80211EnumeratedBand::Ieee80211EnumeratedBand(const char *name, const std::vector<Hz> centers, const std::vector<int> standardChannelNumbers) :
    Ieee80211BandBase(name),
    centers(centers),
    standardChannelNumbers(standardChannelNumbers)
{
    if (!this->standardChannelNumbers.empty() && this->standardChannelNumbers.size() != this->centers.size())
        throw cRuntimeError("Band '%s' has %zu centers but %zu standards channel numbers", name, this->centers.size(), this->standardChannelNumbers.size());
}

Hz Ieee80211EnumeratedBand::getCenterFrequency(int channelNumber) const
{
    if (channelNumber < 0 || channelNumber >= (int)centers.size())
        throw cRuntimeError("Invalid channel number: %d", channelNumber);
    return centers[channelNumber];
}

int Ieee80211EnumeratedBand::getStandardChannelNumber(int channelIndex) const
{
    if (channelIndex < 0 || channelIndex >= (int)centers.size())
        throw cRuntimeError("Invalid channel index: %d", channelIndex);
    if (standardChannelNumbers.empty())
        return IIeee80211Band::getStandardChannelNumber(channelIndex);
    return standardChannelNumbers[channelIndex];
}

int Ieee80211EnumeratedBand::getChannelIndex(int standardChannelNumber) const
{
    if (standardChannelNumbers.empty())
        return IIeee80211Band::getChannelIndex(standardChannelNumber);
    for (int channelIndex = 0; channelIndex < (int)standardChannelNumbers.size(); channelIndex++)
        if (standardChannelNumbers[channelIndex] == standardChannelNumber)
            return channelIndex;
    throw cRuntimeError("Band '%s' has no internal channel index for standards channel number %d", getName(), standardChannelNumber);
}

Ieee80211ArithmeticalBand::Ieee80211ArithmeticalBand(const char *name, Hz start, Hz spacing, int numChannels) :
    Ieee80211BandBase(name),
    start(start),
    spacing(spacing),
    numChannels(numChannels)
{
}

Hz Ieee80211ArithmeticalBand::getCenterFrequency(int channelNumber) const
{
    if (channelNumber < 0 || channelNumber >= numChannels)
        throw cRuntimeError("Invalid channel number: %d", channelNumber);
    return start + spacing / 2 + spacing * channelNumber;
}

const Ieee80211EnumeratedBand Ieee80211CompliantBands::band2_4GHz("2.4 GHz",
{
    GHz(2.412),    // 1
    GHz(2.417),    // 2
    GHz(2.422),    // 3
    GHz(2.427),    // 4
    GHz(2.432),    // 5
    GHz(2.437),    // 6
    GHz(2.442),    // 7
    GHz(2.447),    // 8
    GHz(2.452),    // 9
    GHz(2.457),    // 10
    GHz(2.462),    // 11
    GHz(2.467),    // 12
    GHz(2.472),    // 13
    GHz(2.484),    // 14, this channel is intentionally further away from the previous than the others, see 802.11 specification
},
{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
});

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz("5 GHz", GHz(5), MHz(5), 200);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz20MHz("5 GHz (20 MHz)", GHz(5), MHz(20), 25);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz40MHz("5 GHz (40 MHz)", GHz(5), MHz(40), 12);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz80MHz("5 GHz (80 MHz)", GHz(5), MHz(80), 5);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz160MHz("5 GHz (160 MHz)", GHz(5), MHz(160), 2);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5_9GHz("5.9 GHz", GHz(5.855), MHz(10), 7);

const std::vector<const IIeee80211Band *> Ieee80211CompliantBands::bands = { &band2_4GHz, &band5GHz, &band5GHz20MHz, &band5GHz40MHz, &band5GHz80MHz, &band5GHz160MHz, &band5_9GHz };

const IIeee80211Band *Ieee80211CompliantBands::findBand(const char *name)
{
    for (auto& band : bands)
        if (!strcmp(band->getName(), name))
            return band;
    return nullptr;
}

const IIeee80211Band *Ieee80211CompliantBands::getBand(const char *name)
{
    const IIeee80211Band *band = findBand(name);
    if (band == nullptr)
        throw cRuntimeError("Unknown 802.11 band: '%s'", name);
    else
        return band;
}

} // namespace physicallayer

} // namespace inet

