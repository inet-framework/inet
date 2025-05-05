//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.common;

import inet.common.Module;
import inet.clock.contract.IClock;
import inet.common.packet.recorder.PcapRecorder;

//
// Serves as the base module for all network interfaces.
//
// Display string directives:
// - %i: interface ID
// - %m: MAC address
// - %n: interface name
// - %a: network address
//
module NetworkInterface extends Module
{
    parameters:
        displayStringTextFormat = default("%a (%i)\n%m");
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        pcapRecorder[*].pcapFile = default("results/" + expand("${configname}-") + fullPath() + ".pcap");
        **.clockModule = default(exists(clock) ? absPath(".clock") : "");
        @networkInterface;
        @lifecycleSupport;
        @class(NetworkInterface);
        @display("i=block/ifcard");
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropInterfaceDown](title="packet drops: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNoCarrier](title="packet drops: no carrier"; source=packetDropReasonIsNoCarrier(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                moduleNamePatterns = ".^";
                @display("p=100,100;is=s");
        }
        clock: <default("")> like IClock if typename != "" {
            parameters:
                @display("p=100,200;is=s");
        }
}

