//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// A QoS classifier that assigns a random User Priority. This is useful
// for testing purposes.
//
simple RandomQosClassifier like IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

