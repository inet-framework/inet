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
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef IIeee80211Mode * IIeee80211ModePtr;
}}

class noncobject IIeee80211ModePtr;
class noncobject TransmissionRequest;
class noncobject ReceptionIndication;

//
// Control info attached to a mac frame that is sent down to the ~Ieee80211Radio.
//
class Ieee80211TransmissionRequest extends TransmissionRequest
{
    IIeee80211ModePtr mode;
}

//
// Control info attached to a mac frame that is sent up from the ~Ieee80211Radio.
//
class Ieee80211ReceptionIndication extends ReceptionIndication
{
    IIeee80211ModePtr mode;
    // TODO: remove already inherited fields
    double snr;
    double lossRate;
    double recPow;
    bool airtimeMetric;
    double testFrameDuration;
    double testFrameError;
    int    testFrameSize;
}
