//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Placeholder module that implements the IEEE 802.2 LLC inserter interface but removes itself from the 
// module hierarchy during initialization. Provides a direct connection between input and output gates 
// when LLC header insertion functionality is not needed. Useful in protocol stacks where LLC header 
// creation is optional or when packets should be sent without LLC encapsulation.
//
// @see ~IIeee8022LlcInserter, ~Ieee8022LlcInserter, ~OmittedModule
//
module OmittedIeee8022LlcInserter like IIeee8022LlcInserter
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/inserter,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
