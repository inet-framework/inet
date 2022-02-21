//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/contract/clns/ClnsAddress.h"
}}

namespace inet;

class ClnsAddress
{
    @existingClass;
    @opaque;
    @toString(.str());
    @fromString(ClnsAddress($));
}
