//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.geometry.Geometry;

cplusplus {{
#include "inet/environment/common/PhysicalObject.h"
}}

namespace inet::physicalenvironment;

class PhysicalObject extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    int id;
    Coord position;
    Quaternion orientation;
}
