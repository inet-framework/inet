//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.general.ipv6autoconfiguration;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network Ipv6AutoconfigurationShowcase
{
    @display("bgb=1120,500");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=60,40;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=60,140;is=s");
        }
        scenarioManager: ScenarioManager {
            displayStringTextFormat = "";
            @display("p=60,240;is=s");
        }
        server: StandardHost6 {
            @display("p=940,220");
        }
        router: Router6 {
            @display("p=560,120");
        }
        switch: EthernetSwitch {
            @display("p=560,270");
        }
        host[4]: StandardHost6 {
            @display("p=200,410,row,250");
        }
    connections:
        router.ethg++ <--> Eth100M <--> switch.ethg++;
        router.ethg++ <--> Eth100M <--> server.ethg++;
        for i=0..3 {
            host[i].ethg++ <--> Eth100M <--> switch.ethg++;
        }
}
