//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.common.SimpleModule;
//
// Implements the given module interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialization.
//
module OmittedIeee8022LlcChecker extends SimpleModule like IIeee8022LlcChecker
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/checker,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

