//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.linklayer.contract.IIeee8022Llc;

//
// Implements the IEEE 802.2 Logical Link Control (LLC) protocol, providing
// connection-less data link layer services between the network layer and the
// MAC sublayer. This module handles the basic LLC header processing for both
// incoming and outgoing packets.
//
// The LLC protocol adds a header containing Service Access Point (SAP) identifiers
// that enable protocol multiplexing across the link layer. This allows multiple
// network layer protocols to share the same MAC layer connection. The module
// processes standard LLC headers with DSAP (Destination SAP) and SSAP (Source SAP)
// fields, along with control information.
//
// Key features:
// - Protocol multiplexing using SAP identifiers
// - Support for both LLC Type 1 (unacknowledged connectionless) service
// - Optional protocol registration with the network layer
// - Socket support for direct LLC communication
//
// This simple implementation focuses on the core LLC functionality without
// the additional components found in the more complex ~Ieee8022LlcLayer module.
//
// @see ~IIeee8022Llc, ~Ieee8022LlcLayer, ~Ieee8022LlcHeader
//
simple Ieee8022Llc like IIeee8022Llc
{
    parameters:
        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign the placement of EthernetEncapsulation and LLC modules
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn; // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode) as well
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd) as well
        input lowerLayerIn;
        output lowerLayerOut;
}
