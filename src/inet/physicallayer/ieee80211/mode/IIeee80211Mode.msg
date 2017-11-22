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

cplusplus {{
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const IIeee80211Mode ConstIIeee80211Mode;
typedef const Ieee80211ModeSet ConstIeee80211ModeSet;
typedef const IIeee80211Mode ConstIIeee80211Mode;
}}

class IIeee80211PreambleMode
{
    @existingClass(true);
    simtime_t duration @editable(false);
};

class IIeee80211HeaderMode
{
    @existingClass(true);
    bps netBitrate @editable(false);
    bps grossBitrate @editable(false);
    b Length @editable(false);
    simtime_t duration @editable(false);
//    IModulation *modulation @editable(false);
};

class IIeee80211DataMode
{
    @existingClass(true);
    bps netBitrate @editable(false);
    bps grossBitrate @editable(false);
//    IModulation *modulation @editable(false);
    int numberOfSpatialStreams @editable(false);
};

class IIeee80211Mode extends cObject
{
    @existingClass(true);
    int legacyCwMin @editable(false);
    int legacyCwMax @editable(false);
    string name @editable(false);
    IIeee80211PreambleMode *preambleMode @getter(_getPreambleMode) @editable(false);
    IIeee80211HeaderMode *headerMode @getter(_getHeaderMode) @editable(false);
    IIeee80211DataMode *dataMode @getter(_getDataMode) @editable(false);
    simtime_t slotTime @editable(false);
    simtime_t sifsTime @editable(false);
    simtime_t rifsTime @editable(false);
    simtime_t ccaTime @editable(false);
    simtime_t phyRxStartDelay @editable(false);
    simtime_t rxTxTurnaroundTime @editable(false);
    simtime_t preambleLength @editable(false);
    simtime_t plcpHeaderLength @editable(false);
    int mpduMaxLength @editable(false);
}

class Ieee80211ModeSet extends cObject
{
    @existingClass(true);
    IIeee80211Mode *slowestMode @getter(_getSlowestMode) @editable(false);
    IIeee80211Mode *fastestMode @getter(_getFastestMode) @editable(false);
    IIeee80211Mode *slowestMandatoryMode @getter(_getSlowestMandatoryMode) @editable(false);
    IIeee80211Mode *fastestMandatoryMode @getter(_getFastestMandatoryMode) @editable(false);
    simtime_t sifsTime @editable(false);
    simtime_t slotTime @editable(false);
    simtime_t phyRxStartDelay @editable(false);
    int cwMin @editable(false);
    int cwMax @editable(false);
}

class ConstIIeee80211Mode extends IIeee80211Mode { @existingClass(true); @editable(false); }
class ConstIeee80211ModeSet extends Ieee80211ModeSet { @existingClass(true); @editable(false); }
