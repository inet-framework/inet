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

#include "inet/physicallayer/ieee80211/mode/Ieee80211IRMode.h"
#include "inet/physicallayer/modulation/4PPMModulation.h"
#include "inet/physicallayer/modulation/16PPMModulation.h"

namespace inet {

namespace physicallayer {

Ieee80211IrPreambleMode::Ieee80211IrPreambleMode(int syncSlotLength) :
    syncSlotLength(syncSlotLength)
{
}

Ieee80211IrHeaderMode::Ieee80211IrHeaderMode(const PPMModulationBase *modulation) :
    modulation(modulation)
{
}

Ieee80211IrDataMode::Ieee80211IrDataMode(const PPMModulationBase *modulation) :
    modulation(modulation)
{
}

Ieee80211IrMode::Ieee80211IrMode(const char *name, const Ieee80211IrPreambleMode *preambleMode, const Ieee80211IrHeaderMode *headerMode, const Ieee80211IrDataMode *dataMode) :
    Ieee80211ModeBase(name),
    preambleMode(preambleMode),
    headerMode(headerMode),
    dataMode(dataMode)
{
}

// preamble modes
const Ieee80211IrPreambleMode Ieee80211IrCompliantModes::irPreambleMode64SyncSlots(64);

// header modes
const Ieee80211IrHeaderMode Ieee80211IrCompliantModes::irHeaderMode1Mbps(&_16PPMModulation::singleton);
const Ieee80211IrHeaderMode Ieee80211IrCompliantModes::irHeaderMode2Mbps(&_4PPMModulation::singleton);

// data modes
const Ieee80211IrDataMode Ieee80211IrCompliantModes::irDataMode1Mbps(&_16PPMModulation::singleton);
const Ieee80211IrDataMode Ieee80211IrCompliantModes::irDataMode2Mbps(&_4PPMModulation::singleton);

// modes
const Ieee80211IrMode Ieee80211IrCompliantModes::irMode1Mbps("irMode1Mbps", &irPreambleMode64SyncSlots, &irHeaderMode1Mbps, &irDataMode1Mbps);
const Ieee80211IrMode Ieee80211IrCompliantModes::irMode2Mbps("irMode2Mbps", &irPreambleMode64SyncSlots, &irHeaderMode2Mbps, &irDataMode2Mbps);

const simtime_t Ieee80211IrMode::getRifsTime() const
{
    throw cRuntimeError("Undefined physical layer parameter");
    return SIMTIME_ZERO;
}

} // namespace physicallayer

} // namespace inet

