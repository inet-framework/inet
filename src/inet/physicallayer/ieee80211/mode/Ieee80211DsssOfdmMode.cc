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

#include "inet/physicallayer/ieee80211/mode/Ieee80211DsssOfdmMode.h"

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

