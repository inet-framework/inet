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

#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"

namespace inet {

namespace physicallayer {

Ieee80211DsssDataMode::Ieee80211DsssDataMode(const DPSKModulationBase *modulation) :
    modulation(modulation)
{
}

const simtime_t Ieee80211DsssDataMode::getDuration(int bitLength) const
{
    return (simtime_t)(lrint(ceil(bitLength / getGrossBitrate().get() * 1E+6))) / 1E+6;
}

Ieee80211DsssMode::Ieee80211DsssMode(const Ieee80211DsssPreambleMode *preambleMode, const Ieee80211DsssHeaderMode *headerMode, const Ieee80211DsssDataMode *dataMode) :
    preambleMode(preambleMode),
    headerMode(headerMode),
    dataMode(dataMode)
{
}

// preamble modes
const Ieee80211DsssPreambleMode Ieee80211DsssCompliantModes::dsssPreambleMode1Mbps;

// header modes
const Ieee80211DsssHeaderMode Ieee80211DsssCompliantModes::dsssHeaderMode1Mbps;

// data modes
const Ieee80211DsssDataMode Ieee80211DsssCompliantModes::dsssDataMode1Mbps(&DBPSKModulation::singleton);
const Ieee80211DsssDataMode Ieee80211DsssCompliantModes::dsssDataMode2Mbps(&DQPSKModulation::singleton);

// modes
const Ieee80211DsssMode Ieee80211DsssCompliantModes::dsssMode1Mbps(&dsssPreambleMode1Mbps, &dsssHeaderMode1Mbps, &dsssDataMode1Mbps);
const Ieee80211DsssMode Ieee80211DsssCompliantModes::dsssMode2Mbps(&dsssPreambleMode1Mbps, &dsssHeaderMode1Mbps, &dsssDataMode2Mbps);

} // namespace physicallayer

} // namespace inet

