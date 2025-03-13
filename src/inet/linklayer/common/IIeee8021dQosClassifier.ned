//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Defines the interface for IEEE 802.1d Quality of Service (QoS) classifiers.
// These classifiers analyze packet contents and assign appropriate priority
// levels to ensure differentiated service treatment in the network.
//
// For each packet, the classifier computes an IEEE 802.1d User Priority (UP)
// value ranging from 0 to 7, corresponding to different traffic classes:
// - 1, 2: Background (BK) - lowest priority
// - 0, 3: Best Effort (BE) - default priority
// - 4, 5: Video (VI) - higher priority
// - 6, 7: Voice (VO) - highest priority
//
// The assigned priority is attached to the packet as a tag, which is then
// used by downstream modules (such as queues and MAC protocols) to provide
// appropriate QoS treatment.
//
// @see ~QosClassifier, ~RandomQosClassifier, ~ExampleQosClassifier, ~OmittedIeee8021dQosClassifier
//
moduleinterface IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/fork");
    gates:
        input in;
        output out;
}
