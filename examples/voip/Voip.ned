//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


// Created: 2012-6-10 for project inet

package inet.examples.voip;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
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
                @display("p=250,100");
        }
        receiverHost: StandardHost {
            parameters:
                @display("p=650,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        internetCloud: InternetCloud {
            @display("p=450,100;is=vl");
        }
    connections:
        senderHost.pppg++ <--> C <--> internetCloud.pppg++;
        internetCloud.pppg++ <--> C <--> receiverHost.pppg++;
}
