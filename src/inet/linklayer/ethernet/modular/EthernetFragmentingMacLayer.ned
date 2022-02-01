//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.fragmentation.FragmentTagBasedDefragmenter;

module EthernetFragmentingMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("");
        streamer.typename = default("PreemptableStreamer");
        streamer.minPacketLength = 60B;
        fcsInserter.typename = default("EthernetFragmentFcsInserter");
        phyHeaderInserter.typename = default("EthernetFragmentPhyHeaderInserter");
        transmitter.typename = default("StreamThroughTransmitter");
        fcsChecker.typename = default("EthernetFragmentFcsChecker");
        phyHeaderChecker.typename = default("EthernetFragmentPhyHeaderChecker");
    submodules:
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=500,300");
        }
    connections:
        fcsChecker.out --> { @reconnect; } --> defragmenter.in;
        defragmenter.out --> { @reconnect;@display("m=n"); } --> upperLayerOut;
}
