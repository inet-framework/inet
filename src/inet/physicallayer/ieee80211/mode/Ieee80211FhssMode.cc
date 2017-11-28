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

#include "inet/physicallayer/ieee80211/mode/Ieee80211FhssMode.h"
#include "inet/physicallayer/modulation/2GfskModulation.h"
#include "inet/physicallayer/modulation/4GfskModulation.h"

namespace inet {

namespace physicallayer {

Ieee80211FhssDataMode::Ieee80211FhssDataMode(const GfskModulationBase *modulation) :
    modulation(modulation)
{
}

Ieee80211FhssMode::Ieee80211FhssMode(const char *name, const Ieee80211FhssPreambleMode *preambleMode, const Ieee80211FhssHeaderMode *headerMode, const Ieee80211FhssDataMode *dataMode) :
    Ieee80211ModeBase(name),
    preambleMode(preambleMode),
    headerMode(headerMode),
    dataMode(dataMode)
{
}

// preamble modes
const Ieee80211FhssPreambleMode Ieee80211FhssCompliantModes::fhssPreambleMode1Mbps;

// header modes
const Ieee80211FhssHeaderMode Ieee80211FhssCompliantModes::fhssHeaderMode1Mbps;

// data modes
const Ieee80211FhssDataMode Ieee80211FhssCompliantModes::fhssDataMode1Mbps(&_2GfskModulation::singleton);
const Ieee80211FhssDataMode Ieee80211FhssCompliantModes::fhssDataMode2Mbps(&_4GfskModulation::singleton);

// modes
const Ieee80211FhssMode Ieee80211FhssCompliantModes::fhssMode1Mbps("fhssMode1Mbps", &fhssPreambleMode1Mbps, &fhssHeaderMode1Mbps, &fhssDataMode1Mbps);
const Ieee80211FhssMode Ieee80211FhssCompliantModes::fhssMode2Mbps("fhssMode2Mbps", &fhssPreambleMode1Mbps, &fhssHeaderMode1Mbps, &fhssDataMode2Mbps);

const simtime_t Ieee80211FhssMode::getRifsTime() const
{
    return -1;
}

} // namespace physicallayer

} // namespace inet

