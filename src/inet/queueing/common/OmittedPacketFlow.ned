//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.common.CompoundModule;
import inet.queueing.contract.IPacketFlow;

//
// Implements the given module interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedPacketFlow extends CompoundModule like IPacketFlow
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

