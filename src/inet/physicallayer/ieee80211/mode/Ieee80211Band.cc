//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/ieee80211/mode/Ieee80211Band.h"

namespace inet {

namespace physicallayer {

Ieee80211BandBase::Ieee80211BandBase(const char *name) :
    name(name)
{
}

Ieee80211EnumeratedBand::Ieee80211EnumeratedBand(const char *name, const std::vector<Hz> centers) :
    Ieee80211BandBase(name),
    centers(centers)
{
}

Hz Ieee80211EnumeratedBand::getCenterFrequency(int channelNumber) const
{
    if (channelNumber < 0 || channelNumber >= (int)centers.size())
        throw cRuntimeError("Invalid channel number: %d", channelNumber);
    return centers[channelNumber];
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
});

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz("5 GHz", GHz(5), MHz(5), 200);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz20MHz("5 GHz (20 MHz)", GHz(5), MHz(20), 25);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz40MHz("5 GHz (40 MHz)", GHz(5), MHz(40), 12);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz80MHz("5 GHz (80 MHz)", GHz(5), MHz(80), 5);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5GHz160MHz("5 GHz (160 MHz)", GHz(5), MHz(160), 2);

const Ieee80211ArithmeticalBand Ieee80211CompliantBands::band5_9GHz("5.9 GHz", GHz(5.855), MHz(10), 7);

const std::vector<const IIeee80211Band *> Ieee80211CompliantBands::bands = {&band2_4GHz, &band5GHz, &band5GHz20MHz, &band5GHz40MHz, &band5GHz80MHz, &band5GHz160MHz, &band5_9GHz};

const IIeee80211Band *Ieee80211CompliantBands::findBand(const char *name)
{
    for (auto & band : bands)
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

