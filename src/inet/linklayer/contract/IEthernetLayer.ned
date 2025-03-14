//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Ethernet protocol stack implementations. Provides a standardized
// interface for modules that handle Ethernet frame processing, including MAC header
// insertion/checking, frame padding, and Frame Check Sequence (FCS) calculation/verification.
// Implementations connect to upper layer protocols and lower layer physical interfaces
// through standardized gates.
//
// Typical implementations include:
// - EthernetLayer: A modular implementation with separate components for each function
// - EthernetEncapsulation: Performs Ethernet II or Ethernet with LLC/SNAP encapsulation/decapsulation
//
// @see EthernetLayer, EthernetEncapsulation, OmittedEthernetEncapsulation
//
moduleinterface IEthernetLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
