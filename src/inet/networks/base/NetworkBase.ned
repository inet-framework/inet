//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.base;

import inet.common.packet.recorder.PcapRecorder;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;
import inet.visualizer.contract.IIntegratedVisualizer;

network NetworkBase
{
    parameters:
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        pcapRecorder[*].pcapFile = default("results/" + expand("${configname}-") + fullPath() + ".pcap");
        @class(NetworkBase);
    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                moduleNamePatterns = ".^";
                @display("p=100,100;is=s");
        }
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: <default("Ipv4NetworkConfigurator")> like IL3NetworkConfigurator if typename != "" {
            parameters:
                @display("p=100,300;is=s");
        }
}
