//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.physicallayer.contract.packetlevel.IModulation;

cplusplus {{
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"
}}

namespace inet::physicallayer;

class IIeee80211PreambleMode
{
    @existingClass;
    @descriptor(readonly);
    simtime_t duration;
};

class IIeee80211HeaderMode
{
    @existingClass;
    @descriptor(readonly);
    bps netBitrate;
    bps grossBitrate;
    b Length;
    simtime_t duration;
    IModulation *modulation;
};

class IIeee80211DataMode
{
    @existingClass;
    @descriptor(readonly);
    bps netBitrate;
    bps grossBitrate;
    IModulation *modulation;
    int numberOfSpatialStreams;
};

class IIeee80211Mode extends cObject
{
    @existingClass;
    @descriptor(readonly);
    int legacyCwMin;
    int legacyCwMax;
    string name;
    IIeee80211PreambleMode *preambleMode @getter(_getPreambleMode);
    IIeee80211HeaderMode *headerMode @getter(_getHeaderMode);
    IIeee80211DataMode *dataMode @getter(_getDataMode);
    simtime_t slotTime;
    simtime_t sifsTime;
    simtime_t rifsTime;
    simtime_t ccaTime;
    simtime_t phyRxStartDelay;
    simtime_t rxTxTurnaroundTime;
    simtime_t preambleLength;
    simtime_t plcpHeaderLength;
    int mpduMaxLength;
}

class Ieee80211ModeSet extends cObject
{
    @existingClass;
    @descriptor(readonly);
    IIeee80211Mode *slowestMode @getter(_getSlowestMode);
    IIeee80211Mode *fastestMode @getter(_getFastestMode);
    IIeee80211Mode *slowestMandatoryMode @getter(_getSlowestMandatoryMode);
    IIeee80211Mode *fastestMandatoryMode @getter(_getFastestMandatoryMode);
    simtime_t sifsTime;
    simtime_t slotTime;
    simtime_t phyRxStartDelay;
    int cwMin;
    int cwMax;
}

class Ieee80211OfdmModulation extends IModulation
{
    @existingClass;
    @descriptor(readonly);
    IModulation *subcarrierModulation;
};
