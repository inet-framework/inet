//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Interface for modules that create and insert IEEE 802.2 Logical Link Control (LLC) headers into 
// outgoing packets. Defines the contract for components that prepare packets from upper layers for 
// transmission over link layer technologies using LLC headers. Implementations typically map network 
// layer protocol types to appropriate Service Access Point (SAP) values in the LLC header.
//
// @see ~Ieee8022LlcInserter, ~IIeee8022LlcChecker, ~IIeee8022SnapInserter
//
moduleinterface IIeee8022LlcInserter
{
    parameters:
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}
