//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.common.SimpleModule;
//
// Implements the module given by the interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialization.
//
module OmittedIeee8021qTagEpdHeaderInserter extends SimpleModule like IIeee8021qTagEpdHeaderInserter
{
    parameters:
        string vlanTagType = default("");
        @class(::inet::OmittedModule);
        @display("i=block/inserter,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

