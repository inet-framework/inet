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

#include "inet/physicallayer/ieee80211/mode/Ieee80211ERPOFDMMode.h"

namespace inet {
namespace physicallayer {

Ieee80211ErpOfdmMode::Ieee80211ErpOfdmMode(const char *name, const Ieee80211OFDMPreambleMode *preambleMode, const Ieee80211OFDMSignalMode *signalMode, const Ieee80211OFDMDataMode *dataMode) :
        Ieee80211OFDMMode(name, preambleMode, signalMode, dataMode, MHz(20), MHz(20)) // review the channel spacing
{

}

const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps("erpOfdmMode6Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate13, &Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps("erpOfdmMode9Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate15, &Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps("erpOfdmMode12Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate5, &Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps("erpOfdmMode18Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate7, &Ieee80211OFDMCompliantModes::ofdmDataMode18MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps("erpOfdmMode24Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate9, &Ieee80211OFDMCompliantModes::ofdmDataMode24MbpsCS20MHz);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps("erpOfdmMode36Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate11, &Ieee80211OFDMCompliantModes::ofdmDataMode36Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps("erpOfdmMode48Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate1, &Ieee80211OFDMCompliantModes::ofdmDataMode48Mbps);
const Ieee80211ErpOfdmMode Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps("erpOfdmMode54Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate3, &Ieee80211OFDMCompliantModes::ofdmDataMode54Mbps);

const simtime_t Ieee80211ErpOfdmMode::getRifsTime() const
{
    throw cRuntimeError("Undefined physical layer parameter");
    return SIMTIME_ZERO;
}

} /* namespace physicallayer */
} /* namespace inet */
