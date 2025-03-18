//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.ITrafficConditioner;

//
// Implements the given module interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialization.
//
module OmittedTrafficConditioner like ITrafficConditioner
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/downarrow,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
