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
#include "inet/physicallayer/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const Ieee80211Channel ConstIeee80211Channel;
typedef const IIeee80211Band ConstIIeee80211Band;
}}

class IIeee80211Band
{
    @existingClass(true);
    string name @editable(false);
    Hz centerFrequency[] @sizeGetter(getNumChannels) @editable(false);
}

class Ieee80211EnumeratedBand extends IIeee80211Band
{
    @existingClass(true);
}

class Ieee80211ArithmeticalBand extends IIeee80211Band
{
    @existingClass(true);
}

class Ieee80211Channel
{
    @existingClass(true);
    @descriptor(readonly);
    IIeee80211Band *band @editable(false);
    int channelNumber;
}

class ConstIeee80211Channel extends Ieee80211Channel { @existingClass(true); }
