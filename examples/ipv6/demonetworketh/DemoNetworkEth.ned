//
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.ipv6.demonetworketh;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;


network DemoNetworkEth
{
    parameters:
        int n;
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("is=s;p=100,100");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,200;is=s");
        }
        r1: Router6 {
            @display("p=550,150");
        }
        r2: Router6 {
            @display("p=400,150");
        }
        cli[n]: StandardHost6 {
            @display("p=700,100,col,100");
        }
        srv[n]: StandardHost6 {
            @display("p=250,100,col,100");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> r1.ethg++;
            srv[i].ethg++ <--> ethernetline <--> r2.ethg++;
        }
        r1.ethg++ <--> ethernetline <--> r2.ethg++;
}

