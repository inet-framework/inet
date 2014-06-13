//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.rip.infinitycount;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import ned.DatarateChannel;


network RIPInfinityCountTest
{
    parameters:
        int numRipRouters;
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        cRouter: Router {
            hasRIP = true;
            @display("p=237,192");
        }
        bRouter: Router {
            hasRIP = true;
            @display("p=337,192");
        }
        rRouter[numRipRouters]: Router {
            hasRIP = true;
            @display("p=487,192,ri,100,100");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=59,31");
            config = xml("<config>"
                            +"<interface among='cRouter bRouter' address='10.0.0.x' netmask='255.255.255.x' add-static-route='off'/>"
                            +"<interface among='bRouter rRouter[0]' address='10.1.0.x' netmask='255.255.255.x' add-static-route='off'/>"
                            +"<interface among='rRouter[*]' address='192.168.1.x' netmask='255.255.255.x' add-static-route='off'/>"
                        +"</config>");
        }
        scenarioManager: ScenarioManager {
            @display("p=59,86");
            script = xml("<scenario>"
                            +"<at t='60'>"
                                +"<disconnect src-module='rRouter[0]' src-gate='ethg[0]' />"
                            +"</at>"
                        +"</scenario>");
        }
    connections:
        bRouter.ethg++ <--> C <--> rRouter[0].ethg++;
        cRouter.ethg++ <--> C <--> bRouter.ethg++;        // ha ez a link van, akkor van infinity count
        for i=0..numRipRouters-1 {
            rRouter[i].ethg++ <--> C <--> rRouter[(i+1)%numRipRouters].ethg++;
        }
}

