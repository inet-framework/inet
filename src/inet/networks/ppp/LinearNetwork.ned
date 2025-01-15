//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networks.ppp;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network LinearNetwork extends WiredNetworkBase
{
    submodules:
        client: StandardHost {
            @display("p=350,200");
        }
        router: Router {
            @display("p=550,200");
        }
        server: StandardHost {
            @display("p=750,200");
        }
    connections:
        client.pppg++ <--> { datarate = 100Mbps; delay = 1us; } <--> router.pppg++;
        router.pppg++ <--> { datarate = 100Mbps; delay = 1us; } <--> server.pppg++;
}

