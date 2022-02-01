//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.dhcp;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.inet.StandardHost;
import inet.visualizer.common.IntegratedVisualizer;


network DHCPShutdownReboot
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='server' address='192.168.1.1' netmask='255.255.255.0'/></config>");
                @display("p=100,125;is=s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=100,50;is=s");
        }

        client: StandardHost {
            @display("p=250,150");
        }
        server: StandardHost {
            @display("p=600,150");
        }
    connections:
        client.ethg++ <--> Eth10M <--> server.ethg++;
}
