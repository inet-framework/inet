//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.rip.mixednetwork;

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


//
// TODO Auto-generated network
//
network MixedNetwork
{
    parameters:
        int numCHosts;
        int numRipRouters;
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        cHost[numCHosts]: StandardHost {
            @display("p=40,120,ri,180,180");
        }
        cRouter: Router {
            @display("p=220,300");
        }
        bRouter: Router {
            @display("p=450,100");
        }
        rRouter[numRipRouters]: Router {
            hasRIP = true;
            @display("p=580,200,ri,180,180");
        }
        rHost: StandardHost {
            @display("p=580,641");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=59,31");
            config = xml(
                 "<config>"
                +"<interface among='cHost[*] cRouter' address='10.1.x.x' netmask='255.255.x.x'/>"
                +"<interface among='cRouter bRouter' address='10.0.x.x' netmask='255.255.x.x'/>"
                +"<interface hosts='bRouter' towards='rRouter[0]' address='10.0.x.x' netmask='255.255.x.x'/>"
                +"<interface hosts='rRouter[0]' towards='bRouter' address='10.0.x.x' netmask='255.255.x.x' add-static-route='off'/>"
                +"<interface among='rRouter[*]' address='10.2.x.x' netmask='255.255.x.x' add-static-route='off'/>"
                +"<interface among='rRouter[*] rHost' address='10.2.x.x' netmask='255.255.x.x'/>"
                +"<route hosts='rRouter[0]' destination='10.1.0.0' netmask='255.255.0.0' gateway='bRouter'/>"
                +"</config>");
        }
        scenarioManager: ScenarioManager {
            @display("p=59,100");
        }
        lifecycleController: LifecycleController {
            @display("p=59,150");
        }
    connections:
        cRouter.ethg++ <--> C <--> bRouter.ethg++;
        bRouter.ethg++ <--> C <--> rRouter[0].ethg++;
        for i=0..numCHosts-1 {
            cHost[i].ethg++ <--> C <--> cRouter.ethg++;
        }
        for i=0..numRipRouters-1 {
            rRouter[i].ethg++ <--> C <--> rRouter[(i+1)%numRipRouters].ethg++ if numRipRouters>1;
            rRouter[i].ethg++ <--> C <--> rRouter[(i+2)%numRipRouters].ethg++ if numRipRouters>2;
        }
        rHost.ethg++ <--> C <--> rRouter[numRipRouters/2].ethg++;
}

