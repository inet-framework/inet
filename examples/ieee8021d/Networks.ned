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

package inet.examples.ieee8021d;

import inet.common.scenario.ScenarioManager;
import inet.linklayer.configurator.L2NetworkConfigurator;
import inet.linklayer.ieee8021d.tester.StpTester;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;


network SwitchNetwork
{
    submodules:
        stpTester: StpTester {
            @display("p=100,300;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
        l2NetworkConfigurator: L2NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        switch1: EtherSwitch {
            parameters:
                @display("p=400,100");
            gates:
                ethg[];
        }
        switch2: EtherSwitch {
            parameters:
                @display("p=475,300");
            gates:
                ethg[];
        }
        switch3: EtherSwitch {
            parameters:
                @display("p=550,100");
            gates:
                ethg[];
        }
        switch4: EtherSwitch {
            parameters:
                @display("p=400,200");
            gates:
                ethg[];
        }
        switch5: EtherSwitch {
            parameters:
                @display("p=625,300");
            gates:
                ethg[];
        }
        switch6: EtherSwitch {
            parameters:
                @display("p=325,300");
            gates:
                ethg[];
        }
        switch7: EtherSwitch {
            parameters:
                @display("p=550,200");
            gates:
                ethg[];
        }
    connections:
        switch1.ethg++ <--> Eth1G <--> switch4.ethg++;
        switch1.ethg++ <--> Eth1G <--> switch3.ethg++;
        switch2.ethg++ <--> Eth1G <--> switch4.ethg++;
        switch2.ethg++ <--> Eth1G <--> switch6.ethg++;
        switch2.ethg++ <--> Eth1G <--> switch7.ethg++;
        switch3.ethg++ <--> Eth1G <--> switch7.ethg++;
        switch4.ethg++ <--> Eth1G <--> switch6.ethg++;
        switch4.ethg++ <--> Eth1G <--> switch7.ethg++;
        switch5.ethg++ <--> Eth1G <--> switch7.ethg++;
}

network LargeNet extends SwitchNetwork
{
    submodules:
        host1: EtherHost {
            @display("p=250,200");
        }
        host2: EtherHost {
            @display("p=775,300");
        }

        host3: EtherHost {
            @display("p=700,100");
        }
        host4: EtherHost {
            @display("p=775,470");
        }
        host5: EtherHost {
            @display("p=325,470");
        }
        host6: EtherHost {
            @display("p=325,370");
        }
        switch8: EtherSwitch {
            @display("p=475,370");
        }
        switch9: EtherSwitch {
            @display("p=475,470");
        }
        switch10: EtherSwitch {
            @display("p=625,470");
        }
        switch11: EtherSwitch {
            @display("p=625,370");
        }
    connections:
        switch11.ethg++ <--> Eth1G <--> switch9.ethg++;
        switch11.ethg++ <--> Eth1G <--> switch10.ethg++;
        switch11.ethg++ <--> Eth1G <--> switch5.ethg++;
        switch9.ethg++ <--> Eth1G <--> switch10.ethg++;
        switch8.ethg++ <--> Eth1G <--> switch9.ethg++;
        switch8.ethg++ <--> Eth1G <--> switch11.ethg++;
        switch8.ethg++ <--> Eth1G <--> switch5.ethg++;
        switch8.ethg++ <--> Eth1G <--> switch2.ethg++;
        switch3.ethg++ <--> Eth1G <--> host3.ethg;
        switch5.ethg++ <--> Eth1G <--> host2.ethg;
        switch6.ethg++ <--> Eth1G <--> host1.ethg;
        host5.ethg <--> Eth1G <--> switch9.ethg++;
        host4.ethg <--> Eth1G <--> switch10.ethg++;
        host6.ethg <--> Eth1G <--> switch8.ethg++;
}
