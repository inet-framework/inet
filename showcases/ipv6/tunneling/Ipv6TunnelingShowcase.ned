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
    @display("bgb=1010,395");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=60,40;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=145,40;is=s");
        }
        hostA: StandardHost6 {
            @display("p=85,255");
        }
        borderA: Router6 {
            @display("p=255,185");
        }
        transit: Router6 {
            @display("p=475,105");
        }
        borderB: Router6 {
            @display("p=695,185");
        }
        switchB: EthernetSwitch {
            @display("p=765,300");
        }
        hostB1: StandardHost6 {
            @display("p=935,240");
        }
        hostB2: StandardHost6 {
            @display("p=935,345");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> borderA.ethg++;
        borderA.ethg++ <--> Eth100M <--> transit.ethg++;
        transit.ethg++ <--> Eth100M <--> borderB.ethg++;
        borderB.ethg++ <--> Eth100M <--> switchB.ethg++;
        hostB1.ethg++ <--> Eth100M <--> switchB.ethg++;
        hostB2.ethg++ <--> Eth100M <--> switchB.ethg++;
}
