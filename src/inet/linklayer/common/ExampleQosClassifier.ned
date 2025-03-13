//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Provides an example implementation of the IEEE 802.1d QoS classifier interface
// that demonstrates how to assign User Priority values based on transport layer
// information. This classifier examines packet headers and assigns priorities
// based on the transport protocol type and port numbers.
//
// This module serves as a reference implementation and educational example for
// creating custom QoS classifiers. The actual classification logic is implemented
// in C++ and can be examined to understand how to develop custom classifiers.
//
// Unlike the more configurable QosClassifier, this example has hardcoded rules
// in its C++ implementation, making it suitable for demonstration purposes but
// less flexible for production use.
//
// @see ~IIeee8021dQosClassifier, ~QosClassifier
//
simple ExampleQosClassifier like IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}
