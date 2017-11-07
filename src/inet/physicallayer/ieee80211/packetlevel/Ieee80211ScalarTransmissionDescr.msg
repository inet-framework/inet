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

cplusplus {{
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const IIeee80211Mode  ConstIIeee80211Mode;
typedef const Ieee80211Channel ConstIeee80211Channel;
}}

class ConstIIeee80211Mode extends void;
class ConstIeee80211Channel extends void;
class ScalarTransmission extends cObject;

class Ieee80211ScalarTransmission extends ScalarTransmission
{
    @existingClass(true);
    @descriptor(readonly);
    ConstIIeee80211Mode *mode;
    ConstIeee80211Channel *channel;
}

