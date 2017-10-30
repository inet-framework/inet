//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211ErpOfdmMode.h"

namespace inet {
namespace physicallayer {

Ieee80211ErpOfdmMode::Ieee80211ErpOfdmMode(const char *name, bool isErpOnly, const Ieee80211OfdmPreambleMode *preambleMode, const Ieee80211OfdmSignalMode *signalMode, const Ieee80211OfdmDataMode *dataMode) :
        Ieee80211OfdmMode(name, preambleMode, signalMode, dataMode, MHz(20), MHz(20)), // review the channel spacing
        isErpOnly(isErpOnly)
{

}

const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps("erpOfdmMode6Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13, &Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps("erpOfdmMode9Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate15, &Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps("erpOfdmMode12Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate5, &Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps("erpOfdmMode18Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate7, &Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps("erpOfdmMode24Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate9, &Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps("erpOfdmMode36Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate11, &Ieee80211OfdmCompliantModes::ofdmDataMode36Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps("erpOfdmMode48Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate1, &Ieee80211OfdmCompliantModes::ofdmDataMode48Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps("erpOfdmMode54Mbps", false, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate3, &Ieee80211OfdmCompliantModes::ofdmDataMode54Mbps);

const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode6Mbps("erpOnlyOfdmMode6Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13, &Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode9Mbps("erpOnlyOfdmMode9Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate15, &Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode12Mbps("erpOnlyOfdmMode12Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate5, &Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode18Mbps("erpOnlyOfdmMode18Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate7, &Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode24Mbps("erpOnlyOfdmMode24Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate9, &Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode36Mbps("erpOnlyOfdmMode36Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate11, &Ieee80211OfdmCompliantModes::ofdmDataMode36Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode48Mbps("erpOnlyOfdmMode48Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate1, &Ieee80211OfdmCompliantModes::ofdmDataMode48Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode54Mbps("erpOnlyOfdmMode54Mbps", true, &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate3, &Ieee80211OfdmCompliantModes::ofdmDataMode54Mbps);

const simtime_t Ieee80211ErpOfdmMode::getRifsTime() const
{
    return -1;
}

} /* namespace physicallayer */
} /* namespace inet */
