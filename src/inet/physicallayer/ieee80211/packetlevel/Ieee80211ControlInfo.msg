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
import inet.physicallayer.contract.packetlevel.RadioControlInfo;

cplusplus {{
#include "inet/physicallayer/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const IIeee80211Mode ConstIIeee80211Mode;
typedef const Ieee80211ModeSet ConstIeee80211ModeSet;
typedef const IIeee80211Mode ConstIIeee80211Mode;
}}

class IIeee80211Band
{
    @existingClass;
    string name @editable(false);
    Hz centerFrequency[] @sizeGetter(getNumChannels) @opaque @editable(false);
}

class Ieee80211Channel
{
    @existingClass;
    @descriptor(readonly);
    IIeee80211Band *band @editable(false);
    int channelNumber;
}

class Ieee80211EnumeratedBand extends IIeee80211Band
{
    @existingClass;
}

class Ieee80211ArithmeticalBand extends IIeee80211Band
{
    @existingClass;
}

class IIeee80211PreambleMode
{
    @existingClass;
    simtime_t duration @editable(false);
};

class IIeee80211HeaderMode
{
    @existingClass;
    bps netBitrate @editable(false) @opaque;
    bps grossBitrate @editable(false) @opaque;
    b Length @editable(false) @opaque;
    simtime_t duration @editable(false);
//    IModulation *modulation @editable(false);
};

class IIeee80211DataMode
{
    @existingClass;
    bps netBitrate @editable(false) @opaque;
    bps grossBitrate @editable(false) @opaque;
//    IModulation *modulation @editable(false);
    int numberOfSpatialStreams @editable(false);
};

class IIeee80211Mode extends cObject
{
    @existingClass;
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
    @existingClass;
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

class ConstIIeee80211Mode extends IIeee80211Mode { @existingClass; @editable(false); @descriptor(false); }
class ConstIeee80211ModeSet extends Ieee80211ModeSet { @existingClass; @editable(false); @descriptor(false); }

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class Ieee80211ConfigureRadioCommand extends ConfigureRadioCommand
{
    @descriptor(false);
    string opMode;                         // new default operation mode or "" if not set.
    ConstIeee80211ModeSet *modeSet = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // new default mode set or nullptr if not set.
    ConstIIeee80211Mode *mode = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>"));      // new default transmission mode or nullptr if not set.
    IIeee80211Band *band = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>"));      // new default band or nullptr if not set.
    Ieee80211Channel *channel = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // new default band and channel or nullptr if not set.
    int channelNumber = -1;                // new default channel number in the range [0, numChannels] or -1 if not set.
}

