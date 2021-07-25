//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/clock/contract/ClockTime.h"
}}

import inet.common.INETDefs;

namespace inet;

class clocktime_t
{
    @existingClass;
    @overwritePreviousDefinition;
    @descriptor(false);
    @opaque;
    @byValue;
    @cppType(inet::clocktime_t);
    @fromString(SIMTIME_AS_CLOCKTIME(string2simtime($))); @toString(simtime2string(CLOCKTIME_AS_SIMTIME($)));
    @defaultValue(CLOCKTIME_ZERO);
    @castFunction(false);
    @toValue(cValue($.dbl(),"s"));
}

