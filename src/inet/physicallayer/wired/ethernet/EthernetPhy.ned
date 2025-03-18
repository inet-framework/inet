//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

//
// Represents the physical layer of an Ethernet interface. Handles the conversion
// between Ethernet frames and Ethernet signals. Adds physical headers to outgoing
// frames and encapsulates them in signals for transmission. Decapsulates incoming
// signals, removes physical headers, and forwards the frames to the upper layer.
//
simple EthernetPhy
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        inout phys @labels(EthernetSignal);
}
