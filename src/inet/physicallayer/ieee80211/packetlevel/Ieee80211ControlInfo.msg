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

cplusplus {{
#include "inet/common/Units.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
}}


namespace inet::physicallayer;

class noncobject bps;
class noncobject Hz;

cplusplus {{
typedef const IIeee80211Mode ConstIIeee80211Mode;
typedef const Ieee80211ModeSet * Ieee80211ModeSetPtr;
typedef const IIeee80211Mode * IIeee80211ModePtr;
typedef IIeee80211Band * IIeee80211BandPtr;
typedef Ieee80211Channel * Ieee80211ChannelPtr;

typedef const IIeee80211PreambleMode* IIeee80211PreambleModePtr;
typedef const IIeee80211HeaderMode* IIeee80211HeaderModePtr;
typedef const IIeee80211DataMode* IIeee80211DataModePtr;
}}

class IIeee80211Band
{
    @existingClass;
    string name @editable(false);
    Hz centerFrequency[] @sizeGetter(getNumChannels) @opaque;
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
    int bitLength @editable(false);
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

class noncobject IIeee80211ModePtr;
class noncobject Ieee80211ModeSetPtr;
class noncobject IIeee80211ModePtr;
class noncobject IIeee80211BandPtr;
class noncobject Ieee80211ChannelPtr;
class noncobject ConfigureRadioCommand;
class noncobject TransmissionRequest;
class noncobject ReceptionIndication;
//
// Control info attached to a configure command that is sent to the ~Radio.
//
class Ieee80211ConfigureRadioCommand extends ConfigureRadioCommand
{
    @descriptor(false);
    string opMode;                         // new default operation mode or "" if not set.
    Ieee80211ModeSetPtr modeSet = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // new default mode set or nullptr if not set.
    IIeee80211ModePtr mode = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>"));      // new default transmission mode or nullptr if not set.
    IIeee80211BandPtr band = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>"));      // new default band or nullptr if not set.
    Ieee80211ChannelPtr channel = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // new default band and channel or nullptr if not set.
    int channelNumber = -1;                // new default channel number in the range [0, numChannels] or -1 if not set.
}

//
// Control info attached to a mac frame that is sent down to the ~Ieee80211Radio.
//
class Ieee80211TransmissionRequest extends TransmissionRequest
{
    @descriptor(false);
    IIeee80211ModePtr mode = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>"));      // override default transmission mode or nullptr if not set.
    int channelNumber = -1;                // override default channel or -1 if not set.
    Ieee80211ChannelPtr channel = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // override default band and channel or nullptr if not set.
}

//
// Control info attached to a mac frame that is sent up from the ~Ieee80211Radio.
//
class Ieee80211ReceptionIndication extends ReceptionIndication
{
    @descriptor(false);
    IIeee80211ModePtr mode = nullptr;      // specifies reception mode.
    Ieee80211ChannelPtr channel = nullptr @opaque @tostring($ ? $->getCompleteStringRepresentation() : std::string("<nullptr>")); // specifies reception band and channel
    // TODO: remove already inherited fields
    double snr;
    double lossRate;
    double recPow;
    bool airtimeMetric;
    double testFrameDuration;
    double testFrameError;
    int    testFrameSize;
}

