//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211IrMode.h"

#include "inet/physicallayer/wireless/common/modulation/16PpmModulation.h"
#include "inet/physicallayer/wireless/common/modulation/4PpmModulation.h"

namespace inet {

namespace physicallayer {

Ieee80211IrPreambleMode::Ieee80211IrPreambleMode(int syncSlotLength) :
    syncSlotLength(syncSlotLength)
{
}

Ieee80211IrHeaderMode::Ieee80211IrHeaderMode(const PpmModulationBase *modulation) :
    modulation(modulation)
{
}

Ieee80211IrDataMode::Ieee80211IrDataMode(const PpmModulationBase *modulation) :
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
const Ieee80211IrHeaderMode Ieee80211IrCompliantModes::irHeaderMode1Mbps(&_16PpmModulation::singleton);
const Ieee80211IrHeaderMode Ieee80211IrCompliantModes::irHeaderMode2Mbps(&_4PpmModulation::singleton);

// data modes
const Ieee80211IrDataMode Ieee80211IrCompliantModes::irDataMode1Mbps(&_16PpmModulation::singleton);
const Ieee80211IrDataMode Ieee80211IrCompliantModes::irDataMode2Mbps(&_4PpmModulation::singleton);

// modes
const Ieee80211IrMode Ieee80211IrCompliantModes::irMode1Mbps("irMode1Mbps", &irPreambleMode64SyncSlots, &irHeaderMode1Mbps, &irDataMode1Mbps);
const Ieee80211IrMode Ieee80211IrCompliantModes::irMode2Mbps("irMode2Mbps", &irPreambleMode64SyncSlots, &irHeaderMode2Mbps, &irDataMode2Mbps);

const simtime_t Ieee80211IrMode::getRifsTime() const
{
    return -1;
}

} // namespace physicallayer

} // namespace inet

