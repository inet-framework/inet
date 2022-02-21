//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/linklayer/common/MacAddress.h"
}}

namespace inet;

class MacAddress
{
    @existingClass;
    @opaque;
    @toString(.str());
    @fromString(MacAddress($));
}
