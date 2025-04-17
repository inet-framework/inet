//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

import inet.common.SimpleModule;

//
// A QoS classifier that assigns a random User Priority. This is useful
// for testing purposes.
//
simple RandomQosClassifier extends SimpleModule like IIeee8021dQosClassifier
{
    parameters:
        @class(RandomQosClassifier);
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

