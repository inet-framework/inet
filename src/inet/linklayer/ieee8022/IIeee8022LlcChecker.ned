//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Interface for modules that validate and process IEEE 802.2 Logical Link Control (LLC) headers 
// in incoming packets. Defines the contract for components that extract protocol information from 
// LLC headers, determine the encapsulated protocol, and prepare packets for upper layer processing.
// Implementations typically validate header structure, remove LLC headers, and attach protocol tags.
//
// @see ~Ieee8022LlcChecker, ~IIeee8022LlcInserter, ~IIeee8022SnapChecker
//
moduleinterface IIeee8022LlcChecker
{
    parameters:
        @display("i=block/checker");
    gates:
        input in;
        output out;
}
