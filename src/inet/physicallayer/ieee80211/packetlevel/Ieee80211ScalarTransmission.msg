//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.physicallayer.analogmodel.packetlevel.ScalarTransmission;
import inet.physicallayer.ieee80211.mode.IIeee80211Mode;
import inet.physicallayer.ieee80211.mode.Ieee80211Channel;

cplusplus {{
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
}}

namespace inet::physicallayer;

class Ieee80211ScalarTransmission extends ScalarTransmission
{
    @existingClass;
    @descriptor(readonly);
    const IIeee80211Mode *mode;
    const Ieee80211Channel *channel;
}

