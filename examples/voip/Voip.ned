//
// This program is property of its copyright holder. All rights reserved.
// 

// Created: 2012-6-10 for project inet

package inet.examples.voip;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


//
// A generated network with star topology.
//
network Voip
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 0.1us;
        }
    submodules:
        senderHost: StandardHost {
            parameters:
                @display("p=63,94");
        }
        receiverHost: StandardHost {
            parameters:
                @display("p=260,94");
        }
        iPv4NetworkConfigurator: IPv4NetworkConfigurator {
            @display("p=126,20");
        }
    connections:
        senderHost.ethg++ <--> C <--> receiverHost.ethg++;
}
