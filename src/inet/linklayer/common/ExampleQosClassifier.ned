//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

import inet.common.SimpleModule;

//
// An example QoS classifier that assigns a User Priority based on
// the transport protocol and port numbers.
//
simple ExampleQosClassifier extends SimpleModule like IIeee8021dQosClassifier
{
    parameters:
        @class(ExampleQosClassifier);
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

