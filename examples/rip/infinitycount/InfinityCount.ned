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

package inet.examples.rip.infinitycount;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
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
            hasRip = true;
            @display("p=250,150");
        }
        bRouter: Router {
            hasRip = true;
            @display("p=400,150");
        }
        rRouter[numRipRouters]: Router {
            hasRip = true;
            @display("p=487,192,ri,100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
            config = xml("<config>"
                            +"<interface among='cRouter bRouter' address='10.0.0.x' netmask='255.255.255.x' add-static-route='off'/>"
                            +"<interface among='bRouter rRouter[0]' address='10.1.0.x' netmask='255.255.255.x' add-static-route='off'/>"
                            +"<interface among='rRouter[*]' address='192.168.1.x' netmask='255.255.255.x' add-static-route='off'/>"
                        +"</config>");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
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

