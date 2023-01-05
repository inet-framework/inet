//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// This module is a placeholder that does not perform any function and is removed
// from the module hierarchy (i.e. replaced by a connection) during initialization.
// It can be used instead of making a submodule of the given module interface type
// conditional.
//
module OmittedIeee8022LlcChecker like IIeee8022LlcChecker
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

