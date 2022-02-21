//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

class L3Address
{
    @existingClass;
    @opaque;
    @toString(.str());
}
