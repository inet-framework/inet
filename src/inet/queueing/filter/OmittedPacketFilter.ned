//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.contract.IPacketFilter;

//
// Placeholder module that implements the IPacketFilter interface but removes itself
// from the module hierarchy during initialization, shortcutting its input to its output.
// This module enables flexible network configurations with optional filtering components.
//
module OmittedPacketFilter like IPacketFilter
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/star,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

