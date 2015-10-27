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
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const Ieee80211ModeSet * Ieee80211ModeSetPtr;
typedef const IIeee80211Mode * IIeee80211ModePtr;
typedef IIeee80211Band * IIeee80211BandPtr;
typedef Ieee80211Channel * Ieee80211ChannelPtr;
}}

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
    string opMode;                         // new default operation mode or "" if not set.
    Ieee80211ModeSetPtr modeSet = nullptr; // new default mode set or nullptr if not set.
    IIeee80211ModePtr mode = nullptr;      // new default transmission mode or nullptr if not set.
    IIeee80211BandPtr band = nullptr;      // new default band or nullptr if not set.
    Ieee80211ChannelPtr channel = nullptr; // new default band and channel or nullptr if not set.
    int channelNumber = -1;                // new default channel number in the range [0, numChannels] or -1 if not set.
}

//
// Control info attached to a mac frame that is sent down to the ~Ieee80211Radio.
//
class Ieee80211TransmissionRequest extends TransmissionRequest
{
    IIeee80211ModePtr mode = nullptr;      // override default transmission mode or nullptr if not set.
    int channelNumber = -1;                // override default channel or -1 if not set.
    Ieee80211ChannelPtr channel = nullptr; // override default band and channel or nullptr if not set.
}

//
// Control info attached to a mac frame that is sent up from the ~Ieee80211Radio.
//
class Ieee80211ReceptionIndication extends ReceptionIndication
{
    IIeee80211ModePtr mode = nullptr;      // specifies reception mode.
    Ieee80211ChannelPtr channel = nullptr; // specifies reception band and channel
    // TODO: remove already inherited fields
    double snr;
    double lossRate;
    double recPow;
    bool airtimeMetric;
    double testFrameDuration;
    double testFrameError;
    int    testFrameSize;
}
