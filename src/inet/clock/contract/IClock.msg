//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

cplusplus {{
#include "inet/clock/contract/IClock.h"
}}

import inet.common.INETDefs;

namespace inet;

class IClock {
    @existingClass;
    @descriptor(false);
    @cppType(inet::IClock);
}
