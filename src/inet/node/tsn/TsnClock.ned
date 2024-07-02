//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.linklayer.ieee8021as.GptpMaster;

//
// This module represents a hardware device containing a high-precision hardware
// clock. The device also contains a gPTP protocol implementation and acts as a
// gPTP master node on the network.
//
module TsnClock extends GptpMaster
{
    parameters:
        bool hasCutthroughSwitching = default(false); // Enable cut-through switching support
        clock.typename = default("OscillatorBasedClock"); // Master clocks cannot be set
        ethernet.typename = default("EthernetLayer"); // Use the Ethernet protocol layer outside network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // Switch to modular Ethernet interface
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamingPhyLayer" : "EthernetPhyLayer"); // Use packet streaming when cut-through switching is enabled
        @display("i=device/card"); // Change icon to emphasize hardware device
}
