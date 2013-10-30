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
import inet.nodes.ethernet.EtherHost;
import inet.base.LifecycleController;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.scenario.ScenarioManager;
import inet.linklayer.configurator.L2NetworkConfigurator;

network CompleteNetwork
{
    @display("bgb=689,368");
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=75,101");
        }
        l2NetworkConfigurator: L2NetworkConfigurator {
            @display("p=75,217");
        }
        lifecycleController: LifecycleController {
            @display("p=75,155");
        }

        switchBEB1: EtherSwitch {
            parameters:
                @display("p=333,78");
            gates:
                ethg[];
        }

        switchBEB2: EtherSwitch {
            parameters:
                @display("p=301,217");
            gates:
                ethg[];
        }

        switchBEB3: EtherSwitch {
            parameters:
                @display("p=443,78");
            gates:
                ethg[];
        }

        switchBEB4: EtherSwitch {
            parameters:
                @display("p=322,153");
            gates:
                ethg[];
        }

        switchBEB5: EtherSwitch {
            parameters:
                @display("p=529,217");
            gates:
                ethg[];
        }

        switchBEB6: EtherSwitch {
            parameters:
                @display("p=148,217");
            gates:
                ethg[];
        }

        switchBEB7: EtherSwitch {
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

network NetworkWithHosts extends CompleteNetwork
{
    submodules:
        host1: EtherHost {
            @display("p=187,147");
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

network LargeNet extends NetworkWithHosts
{
    @display("bgb=952,472");
    submodules:
        host4: EtherHost {
            @display("p=679,422");
        }
        host5: EtherHost {
            @display("p=479,422");
        }
        host6: EtherHost {
            @display("p=322,321");
        }
        switchBEB8: EtherSwitch {
            @display("p=443,282");
        }
        switchBEB9: EtherSwitch {
            @display("p=518,352");
        }
        switchBEB10: EtherSwitch {
            @display("p=648,352");
        }
        switchBEB11: EtherSwitch {
            @display("p=579,288");
        }
    connections:
        switchBEB11.ethg++ <--> Eth1G <--> switchBEB9.ethg++;
        switchBEB11.ethg++ <--> Eth1G <--> switchBEB10.ethg++;
        switchBEB11.ethg++ <--> Eth1G <--> switchBEB5.ethg++;
        switchBEB9.ethg++ <--> Eth1G <--> switchBEB10.ethg++;
        switchBEB8.ethg++ <--> Eth1G <--> switchBEB9.ethg++;
        switchBEB8.ethg++ <--> Eth1G <--> switchBEB11.ethg++;
        switchBEB8.ethg++ <--> Eth1G <--> switchBEB5.ethg++;
        switchBEB8.ethg++ <--> Eth1G <--> switchBEB2.ethg++;
        host5.ethg <--> Eth1G <--> switchBEB9.ethg++;
        host4.ethg <--> Eth1G <--> switchBEB10.ethg++;
        host6.ethg <--> Eth1G <--> switchBEB8.ethg++;
}
