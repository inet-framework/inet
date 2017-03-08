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
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
}}

namespace inet::physicallayer;

class noncobject bps;
class NarrowbandTransmissionBase extends cObject;

class FlatTransmissionBase extends NarrowbandTransmissionBase
{
    @existingClass(true);
    @descriptor(readonly);
    int headerBitLength;
    int dataBitLength;
    bps bitrate @opaque;
}
