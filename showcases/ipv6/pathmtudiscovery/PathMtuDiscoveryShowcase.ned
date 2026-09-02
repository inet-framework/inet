//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.ipv6.pathmtudiscovery;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network PathMtuDiscoveryShowcase
{
    @display("bgb=1080,300");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=60,40;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=145,40;is=s");
        }
        hostA: StandardHost6 {
            @display("p=80,190");
        }
        firewall: Router6 {
            @display("p=250,190");
        }
        borderA: Router6 {
            @display("p=420,190");
        }
        transit: Router6 {
            @display("p=590,120");
        }
        borderB: Router6 {
            @display("p=760,190");
        }
        hostB: StandardHost6 {
            @display("p=940,190");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> firewall.ethg++;
        firewall.ethg++ <--> Eth100M <--> borderA.ethg++;
        borderA.ethg++ <--> Eth100M <--> transit.ethg++;
        transit.ethg++ <--> Eth100M <--> borderB.ethg++;
        borderB.ethg++ <--> Eth100M <--> hostB.ethg++;
}
