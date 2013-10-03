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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.rstp;

import inet.nodes.ethernet.Eth1G;
import inet.nodes.rstp.RSTPSwitch;
import inet.nodes.ethernet.EtherHost;
import inet.base.LifecycleController;
import inet.world.scenario.ScenarioManager;

network CompleteNetwork
{
    @display("bgb=689,368");
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=75,101");
        }
        
        lifecycleController: LifecycleController {
            @display("p=75,155");
        }
        
        switchBEB1: RSTPSwitch {
            parameters:
                @display("p=333,78");
            gates:
                ethg[];
        }
        
        switchBEB2: RSTPSwitch {
            parameters:
                @display("p=301,217");
            gates:
                ethg[];
        }
        
        switchBEB3: RSTPSwitch {
            parameters:
                @display("p=443,78");
            gates:
                ethg[];
        }
        
        switchBEB4: RSTPSwitch {
            parameters:
                @display("p=322,153");
            gates:
                ethg[];
        }
        
        switchBEB5: RSTPSwitch {
            parameters:
                @display("p=529,217");
            gates:
                ethg[];
        }
        
        switchBEB6: RSTPSwitch {
            parameters:
                @display("p=148,217");
            gates:
                ethg[];
        }
        
        switchBEB7: RSTPSwitch {
            parameters:
                @display("p=443,153");
            gates:
                ethg[];
        }
        
    connections:
        switchBEB1.ethg++ <--> Eth1G <--> switchBEB4.ethg++;
        switchBEB1.ethg++ <--> Eth1G <--> switchBEB3.ethg++;
        switchBEB2.ethg++ <--> Eth1G <--> switchBEB4.ethg++;
        switchBEB2.ethg++ <--> Eth1G <--> switchBEB6.ethg++;
        switchBEB2.ethg++ <--> Eth1G <--> switchBEB7.ethg++;
        switchBEB3.ethg++ <--> Eth1G <--> switchBEB7.ethg++;
        switchBEB4.ethg++ <--> Eth1G <--> switchBEB6.ethg++;
        switchBEB4.ethg++ <--> Eth1G <--> switchBEB7.ethg++;
        switchBEB5.ethg++ <--> Eth1G <--> switchBEB7.ethg++;
}


network NetworkWithBK extends CompleteNetwork
{
    connections:
        switchBEB5.ethg++ <--> Eth1G <--> switchBEB7.ethg++;
        switchBEB2.ethg++ <--> Eth1G <--> switchBEB2.ethg++;
}


network NetworkWithHosts extends CompleteNetwork
{
    submodules:
        host1: EtherHost {
            @display("p=77,217");
        }
        
        host2: EtherHost {
            @display("p=609,217");
        }
        
        host3: EtherHost {
            @display("p=529,78");
        }
        
    connections:
        switchBEB3.ethg++ <--> Eth1G <--> host3.ethg;
        switchBEB5.ethg++ <--> Eth1G <--> host2.ethg;
        switchBEB6.ethg++ <--> Eth1G <--> host1.ethg;
}
