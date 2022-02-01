//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"

namespace inet {

namespace physicallayer {

Ieee80211DsssDataMode::Ieee80211DsssDataMode(const DpskModulationBase *modulation) :
    modulation(modulation)
{
}

const simtime_t Ieee80211DsssDataMode::getDuration(b length) const
{
    return (simtime_t)(lrint(ceil((double)length.get() / getGrossBitrate().get() * 1E+6))) / 1E+6;
}

Ieee80211DsssMode::Ieee80211DsssMode(const char *name, const Ieee80211DsssPreambleMode *preambleMode, const Ieee80211DsssHeaderMode *headerMode, const Ieee80211DsssDataMode *dataMode) :
    Ieee80211ModeBase(name),
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
const Ieee80211DsssDataMode Ieee80211DsssCompliantModes::dsssDataMode1Mbps(&DbpskModulation::singleton);
const Ieee80211DsssDataMode Ieee80211DsssCompliantModes::dsssDataMode2Mbps(&DqpskModulation::singleton);

// modes
const Ieee80211DsssMode Ieee80211DsssCompliantModes::dsssMode1Mbps("dsssMode1Mbps", &dsssPreambleMode1Mbps, &dsssHeaderMode1Mbps, &dsssDataMode1Mbps);
const Ieee80211DsssMode Ieee80211DsssCompliantModes::dsssMode2Mbps("dsssMode2Mbps", &dsssPreambleMode1Mbps, &dsssHeaderMode1Mbps, &dsssDataMode2Mbps);

const simtime_t Ieee80211DsssMode::getRifsTime() const
{
    return -1;
}

} // namespace physicallayer

} // namespace inet

