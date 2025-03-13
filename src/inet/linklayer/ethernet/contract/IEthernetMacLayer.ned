//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.contract;

//
// Defines the interface for Ethernet MAC layer implementations in the INET framework.
// The MAC (Medium Access Control) layer is responsible for managing access to the
// physical medium and handling frame transmission and reception.
//
// This interface specifies the minimum requirements for any Ethernet MAC layer
// implementation, including the necessary gates for connecting to upper and lower
// layers. The upper layer typically connects to LLC or directly to the network layer,
// while the lower layer connects to the physical layer.
//
// Modules implementing this interface handle various MAC layer functions such as:
// - Frame encapsulation and decapsulation
// - Addressing (source and destination MAC addresses)
// - Error detection using Frame Check Sequence (FCS)
// - Medium access control (e.g., CSMA/CD for half-duplex operation)
// - Inter-frame gap management
//
// @see ~EthernetMacLayer, ~EthernetLayer
//
moduleinterface IEthernetMacLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
