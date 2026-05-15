//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.general.dhcp;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network DhcpShowcase
{
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,50;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='dhcpServer' address='192.168.1.1' netmask='255.255.255.0'/></config>");
                @display("p=100,125;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
        dhcpServer: StandardHost {
            @display("p=400,100");
        }
        switch: EthernetSwitch {
            @display("p=400,250");
        }
        client[3]: StandardHost {
            @display("p=200,400,row,200");
        }
    connections:
        dhcpServer.ethg++ <--> Eth100M <--> switch.ethg++;
        for i=0..2 {
            client[i].ethg++ <--> Eth100M <--> switch.ethg++;
        }
}
