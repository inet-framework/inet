//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;

cplusplus {{
#include "inet/environment/common/Material.h"
}}

namespace inet::physicalenvironment;

class Material extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    Ohmm resistivity;
    double relativePermittivity;
    double relativePermeability;
}
