//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Placeholder module that implements the IEEE 802.2 SNAP checker interface but removes itself from the 
// module hierarchy during initialization. Provides a direct connection between input and output gates 
// when SNAP header checking functionality is not needed. Useful in protocol stacks where SNAP header 
// processing is optional or when packets are known to not contain SNAP headers.
//
// @see ~IIeee8022SnapChecker, ~Ieee8022SnapChecker, ~OmittedModule
//
module OmittedIeee8022SnapChecker like IIeee8022SnapChecker
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
