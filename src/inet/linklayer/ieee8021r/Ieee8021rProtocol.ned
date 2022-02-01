//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

//
// This module implements the IEEE 802.1r protocol encapsulation/decapsulation.
//
module Ieee8021rProtocol like IIeee8021rLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        rTagHeaderInserter: <default("Ieee8021rTagEpdHeaderInserter")> like IIeee8021rTagEpdHeaderInserter {
            @display("p=300,200");
        }
        rTagHeaderChecker: <default("Ieee8021rTagEpdHeaderChecker")> like IIeee8021rTagEpdHeaderChecker {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> rTagHeaderInserter.in;
        rTagHeaderInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> rTagHeaderChecker.in;
        rTagHeaderChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
