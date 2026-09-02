//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.ipv6.tunneling;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network Ipv6TunnelingShowcase
{
    @display("bgb=1180,420");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=70,50;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=70,140;is=s");
        }
        hostA: StandardHost6 {
            @display("p=200,300");
        }
        borderA: Router6 {
            @display("p=380,200");
        }
        transit: Router6 {
            @display("p=590,120");
        }
        borderB: Router6 {
            @display("p=800,200");
        }
        switchB: EthernetSwitch {
            @display("p=950,300");
        }
        hostB1: StandardHost6 {
            @display("p=1090,230");
        }
        hostB2: StandardHost6 {
            @display("p=1090,360");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> borderA.ethg++;
        borderA.ethg++ <--> Eth100M <--> transit.ethg++;
        transit.ethg++ <--> Eth100M <--> borderB.ethg++;
        borderB.ethg++ <--> Eth100M <--> switchB.ethg++;
        hostB1.ethg++ <--> Eth100M <--> switchB.ethg++;
        hostB2.ethg++ <--> Eth100M <--> switchB.ethg++;
}
