//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common.misc;

simple ModuleDump
{
    parameters:
        string modulePath = default("^");
        string fileName = default("");
        double time = default(0);
        bool printClassNames = default(false);
        bool printProperties = default(false);
        bool printParamAssignmentLocations = default(false);
        @display("i=block/star");
}
