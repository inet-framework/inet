// Copyright (C) 2012 OpenSim Ltd
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
// @author: Zoltan Bojthe
//


#include "Ieee80211DataRate.h"

#include "WifiMode.h"


/* Bit rates for 802.11b/g/a/p.
 * Must be ordered by mode, bitrate.
 */
const Ieee80211Descriptor ieee80211Descriptor[] =
{
    {'a',  6000000, WifiModulationType::GetOfdmRate6Mbps()},
    {'a',  9000000, WifiModulationType::GetOfdmRate9Mbps()},
    {'a', 12000000, WifiModulationType::GetOfdmRate12Mbps()},
    {'a', 18000000, WifiModulationType::GetOfdmRate18Mbps()},
    {'a', 24000000, WifiModulationType::GetOfdmRate24Mbps()},
    {'a', 36000000, WifiModulationType::GetOfdmRate36Mbps()},
    {'a', 48000000, WifiModulationType::GetOfdmRate48Mbps()},
    {'a', 54000000, WifiModulationType::GetOfdmRate54Mbps()},

    {'b',  1000000, WifiModulationType::GetDsssRate1Mbps()},
    {'b',  2000000, WifiModulationType::GetDsssRate2Mbps()},
    {'b',  5500000, WifiModulationType::GetDsssRate5_5Mbps()},
    {'b', 11000000, WifiModulationType::GetDsssRate11Mbps()},

    {'g',  1000000, WifiModulationType::GetDsssRate1Mbps()},
    {'g',  2000000, WifiModulationType::GetDsssRate2Mbps()},
    {'g',  5500000, WifiModulationType::GetDsssRate5_5Mbps()},
    {'g',  6000000, WifiModulationType::GetErpOfdmRate6Mbps()},
    {'g',  9000000, WifiModulationType::GetErpOfdmRate9Mbps()},
    {'g', 11000000, WifiModulationType::GetDsssRate11Mbps()},
    {'g', 12000000, WifiModulationType::GetErpOfdmRate12Mbps()},
    {'g', 18000000, WifiModulationType::GetErpOfdmRate18Mbps()},
    {'g', 24000000, WifiModulationType::GetErpOfdmRate24Mbps()},
    {'g', 36000000, WifiModulationType::GetErpOfdmRate36Mbps()},
    {'g', 48000000, WifiModulationType::GetErpOfdmRate48Mbps()},
    {'g', 54000000, WifiModulationType::GetErpOfdmRate54Mbps()},

    {'p',  3000000, WifiModulationType::GetOfdmRate3MbpsBW10MHz()},
    {'p',  4500000, WifiModulationType::GetOfdmRate4_5MbpsBW10MHz()},
    {'p',  6000000, WifiModulationType::GetOfdmRate6MbpsBW10MHz()},
    {'p',  9000000, WifiModulationType::GetOfdmRate9MbpsBW10MHz()},
    {'p', 12000000, WifiModulationType::GetOfdmRate12MbpsBW10MHz()},
    {'p', 18000000, WifiModulationType::GetOfdmRate18MbpsBW10MHz()},
    {'p', 24000000, WifiModulationType::GetOfdmRate24MbpsBW10MHz()},
    {'p', 27000000, WifiModulationType::GetOfdmRate27MbpsBW10MHz()},

    {'\0', 0, ModulationType()} // END
};

