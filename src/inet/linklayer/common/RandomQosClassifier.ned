//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Implements an IEEE 802.1d QoS classifier that assigns random User Priority
// values to packets. This classifier ignores packet contents and simply assigns
// a randomly selected priority level to each packet that passes through it.
//
// The random assignment makes this classifier particularly useful for:
// - Testing QoS mechanisms under varying traffic conditions
// - Evaluating the performance impact of different priority distributions
// - Verifying that downstream modules correctly handle all priority levels
// - Stress-testing priority-based queuing systems
//
// This classifier should only be used for testing and simulation experiments,
// not for modeling real-world QoS behavior where classification is based on
// actual packet characteristics.
//
// @see ~IIeee8021dQosClassifier, ~QosClassifier
//
simple RandomQosClassifier like IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}
