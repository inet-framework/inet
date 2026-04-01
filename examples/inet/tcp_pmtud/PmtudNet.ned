//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.inet.tcp_pmtud;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

//
// Demonstrates TCP Path MTU Discovery (RFC 1191).
//
// Topology:  client --- router1 ---[bottleneck]--- router2 --- server
//
// The bottleneck link between the two routers has a smaller MTU (default 800 B)
// than the access links (1500 B).  When PMTUD is enabled, the client sets the
// DF bit, receives ICMP Fragmentation Needed from router1, and reduces its MSS.
// After the PMTUD timeout the original MSS is restored to probe for a larger
// path MTU.
//
network PmtudNet
{
    parameters:
        int bottleneckMtu @unit(B) = default(800B);
    types:
        channel access extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 1ms;
        }
        channel bottleneck extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 5ms;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,40;is=s");
        }
        client: StandardHost {
            @display("p=100,200;i=device/pc3");
        }
        router1: Router {
            @display("p=250,200");
        }
        router2: Router {
            @display("p=400,200");
        }
        server: StandardHost {
            @display("p=550,200;i=device/server");
        }
    connections:
        client.pppg++  <--> access     <--> router1.pppg++;
        router1.pppg++ <--> bottleneck <--> router2.pppg++;
        router2.pppg++ <--> access     <--> server.pppg++;
}
