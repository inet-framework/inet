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
#include "inet/common/clock/common/ClockTime.h"
}}

namespace inet;

class ClockTime
{
    @existingClass;
    @overwritePreviousDefinition;
    @descriptor(false);
    @opaque;
    @byValue;
    @cppType(inet::common::ClockTime);
    @fromString(inet::common::ClockTime::parse($)); @toString($.str());
    @defaultValue(CLOCKTIME_ZERO);
}

class clocktime_t
{
    @existingClass;
    @overwritePreviousDefinition;
    @descriptor(false);
    @opaque;
    @byValue;
    @cppType(inet::clocktime_t);
    @fromString(inet::common::ClockTime::parse($)); @toString($.str());
    @defaultValue(CLOCKTIME_ZERO);
}
