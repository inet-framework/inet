//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.contract;

//
// Module interface for Ethernet encapsulation/decapsulation.
//
moduleinterface IEthernetEncapsulation
{
    parameters:
        @omittedTypename(OmittedEthernetEncapsulation);
        @display("i=block/square");
    gates:
        input upperLayerIn @labels(Ieee802Ctrl/down); // higher layer
        output upperLayerOut @labels(Ieee802Ctrl/up); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
}

