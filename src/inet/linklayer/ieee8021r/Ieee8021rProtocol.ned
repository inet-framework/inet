//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.linklayer.contract.IIeee8021rLayer;

//
// Implements the core functionality of IEEE 802.1CB Frame Replication and Elimination
// for Reliability (FRER) protocol. This module handles the encapsulation and
// decapsulation of frames with IEEE 802.1r tags, which contain sequence numbers
// essential for duplicate frame identification and elimination.
//
// The protocol operates by:
// - Adding sequence numbers to outgoing frames via the R-TAG header
// - Processing incoming frames to extract sequence information
// - Supporting the identification of duplicate frames based on sequence numbers
// - Working in conjunction with stream identification and filtering functions
//
// The module consists of two main components:
// - R-TAG header inserter: Adds IEEE 802.1r tags with sequence numbers to outgoing frames
// - R-TAG header checker: Processes incoming frames, extracts sequence information,
//   and supports duplicate elimination
//
// The IEEE 802.1r tag format includes a Tag Protocol Identifier (TPID) of 0xF1C1,
// a reserved field, and a sequence number field that enables the receiving end to
// identify duplicate frames that have traversed different network paths.
//
// This implementation uses the EtherType Protocol Discrimination (EPD) format for
// efficient processing in Ethernet networks.
//
// @see ~IIeee8021rTagEpdHeaderInserter, ~IIeee8021rTagEpdHeaderChecker
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
