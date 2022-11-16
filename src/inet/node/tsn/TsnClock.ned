//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.linklayer.ieee8021as.GptpMaster;

//
// This module represents a hardware device containing a high precision hardware
// clock. The device also contains a gPTP protocol implementation and acts as a
// gPTP master node in the network.
//
module TsnClock extends GptpMaster
{
    parameters:
        bool hasCutthroughSwitching = default(false); // enable cut-through switching support
        clock.typename = default("OscillatorBasedClock"); // master clocks cannot be set
        ethernet.typename = default("EthernetLayer"); // use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // switch to modular Ethernet interface
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamingPhyLayer" : "EthernetPhyLayer"); // use packet streaming when cut-through switching is enabled
        @display("i=device/card"); // change icon to emphasise hardware device
}
