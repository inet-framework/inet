//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.contract;

//
// Module interface for IEEE 802.2 Logical Link Control (LLC) protocol
// implementations. Provides protocol multiplexing at the link layer through
// Service Access Points (SAPs). Supports both standard LLC headers with DSAP/SSAP
// fields and SNAP (SubNetwork Access Protocol) headers for EtherType
// encapsulation. Enables multiple higher-layer protocols to share the same
// physical medium. Implementations handle encapsulation/decapsulation of frames
// and can provide socket-based access to LLC services for applications.
//
moduleinterface IIeee8022Llc
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
