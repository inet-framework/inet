//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssOfdmMode.h"

namespace inet {

namespace physicallayer {

Ieee80211DsssOfdmMode::Ieee80211DsssOfdmMode(const char *name, const Ieee80211DsssPreambleMode *dsssPreambleMode, const Ieee80211DsssHeaderMode *dsssHeaderMode, const Ieee80211OfdmPreambleMode *ofdmPreambleMode, const Ieee80211OfdmSignalMode *ofdmSignalMode, const Ieee80211OfdmDataMode *ofdmDataMode) :
    Ieee80211ModeBase(name),
    dsssPreambleMode(dsssPreambleMode),
    dsssHeaderMode(dsssHeaderMode),
    ofdmPreambleMode(ofdmPreambleMode),
    ofdmSignalMode(ofdmSignalMode),
    ofdmDataMode(ofdmDataMode)
{
}

const simtime_t Ieee80211DsssOfdmMode::getRifsTime() const
{
    return -1;
}

} // namespace physicallayer

} // namespace inet

