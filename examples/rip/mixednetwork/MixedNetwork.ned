//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.examples.rip.mixednetwork;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
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
            @display("p=250,100,ri,150,150");
        }
        cRouter: Router {
            @display("p=400,250");
        }
        bRouter: Router {
            @display("p=700,100");
        }
        rRouter[numRipRouters]: Router {
            hasRip = true;
            @display("p=850,100,ri,150,150");
        }
        rHost: StandardHost {
            @display("p=1000,500");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
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
            @display("p=100,200;is=s");
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
        rHost.ethg++ <--> C <--> rRouter[int(numRipRouters/2)].ethg++;
}

