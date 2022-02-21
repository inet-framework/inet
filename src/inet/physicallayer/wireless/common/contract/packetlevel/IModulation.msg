//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IApskModulation.h"
}}

namespace inet::physicallayer;

class IModulation
{
    @existingClass;
    @descriptor(readonly);
};

class IApskModulation
{
    @existingClass;
    @descriptor(readonly);
    unsigned int codeWordSize;
    unsigned int constellationSize;
};
