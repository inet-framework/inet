//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// This module implements the given module interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialization.
//
module OmittedIeee8021dQosClassifier like IIeee8021dQosClassifier
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/classifier,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

