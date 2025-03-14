//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Placeholder module that implements the IEEE 802.2 LLC checker interface but removes itself from the 
// module hierarchy during initialization. Provides a direct connection between input and output gates 
// when LLC header checking functionality is not needed. Useful in protocol stacks where LLC header 
// processing is optional or when packets are known to not contain LLC headers.
//
// @see ~IIeee8022LlcChecker, ~Ieee8022LlcChecker, ~OmittedModule
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
