//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
