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
    @display("bgb=1180,330");
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=60,40;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=145,40;is=s");
        }
        hostA: StandardHost6 {
            @display("p=80,240");
        }
        firewall: Router6 {
            @display("p=290,150");
        }
        borderA: Router6 {
            @display("p=500,240");
        }
        transit: Router6 {
            @display("p=690,150");
        }
        borderB: Router6 {
            @display("p=890,240");
        }
        hostB: StandardHost6 {
            @display("p=1090,150");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> firewall.ethg++;
        firewall.ethg++ <--> Eth100M <--> borderA.ethg++;
        borderA.ethg++ <--> Eth100M <--> transit.ethg++;
        transit.ethg++ <--> Eth100M <--> borderB.ethg++;
        borderB.ethg++ <--> Eth100M <--> hostB.ethg++;
}
