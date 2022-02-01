//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Interface for 802.1d QoS classifiers. For each packet, the classifier
// computes a 802.1d User Priority (UP) value, and sets in on the
// Iee802Ctrl control info before sending out the packet on the "out" gate.
//
moduleinterface IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/fork");
    gates:
        input in;
        output out;
}

