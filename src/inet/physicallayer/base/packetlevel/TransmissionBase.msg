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
import inet.common.geometry.Geometry;

cplusplus {{
#include "inet/physicallayer/base/packetlevel/TransmissionBase.h"
}}

namespace inet::physicallayer;

class TransmissionBase extends cObject
{
    @existingClass;
    @descriptor(readonly);
    int id;
    simtime_t startTime;
    Coord startPosition;
    Quaternion startOrientation;
    simtime_t endTime;
    Coord endPosition;
    Quaternion endOrientation;
    simtime_t duration;
    simtime_t preambleDuration;
    simtime_t headerDuration;
    simtime_t dataDuration;
}
