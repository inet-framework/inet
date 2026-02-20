//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.tutorials.stp;

import inet.common.scenario.ScenarioManager;
import inet.linklayer.configurator.common.L2NetworkConfigurator;
import inet.linklayer.ieee8021d.tester.StpTester;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetHost;
import inet.node.ethernet.EthernetSwitch;

//
// Three Ethernet switches connected in a ring with two hosts.
// Used in Step 1 to demonstrate broadcast loops without STP.
//
network LoopNetwork
{
    submodules:
        switch1: EthernetSwitch {
            @display("p=300,200");
        }
        switch2: EthernetSwitch {
            @display("p=500,200");
        }
        switch3: EthernetSwitch {
            @display("p=400,350");
        }
        host1: EthernetHost {
            @display("p=100,200");
        }
        host2: EthernetHost {
            @display("p=600,350");
        }
    connections:
        switch1.ethg++ <--> Eth1G <--> switch2.ethg++;
        switch2.ethg++ <--> Eth1G <--> switch3.ethg++;
        switch3.ethg++ <--> Eth1G <--> switch1.ethg++;
        host1.ethg    <--> Eth1G <--> switch1.ethg++;
        host2.ethg    <--> Eth1G <--> switch3.ethg++;
}

//
// Seven Ethernet switches with redundant links, forming a looped topology.
// Used in Steps 2-5 to demonstrate STP and RSTP tree building.
//
network SwitchNetwork
{
    submodules:
        stpTester: StpTester {
            @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
        l2NetworkConfigurator: L2NetworkConfigurator {
            @display("p=100,300;is=s");
        }
        switch1: EthernetSwitch {
            @display("p=400,100");
        }
        switch2: EthernetSwitch {
            @display("p=475,300");
        }
        switch3: EthernetSwitch {
            @display("p=550,100");
        }
        switch4: EthernetSwitch {
            @display("p=400,200");
        }
        switch5: EthernetSwitch {
            @display("p=625,300");
        }
        switch6: EthernetSwitch {
            @display("p=325,300");
        }
        switch7: EthernetSwitch {
            @display("p=550,200");
        }
        host1: EthernetHost {
            @display("p=200,300");
        }
        host2: EthernetHost {
            @display("p=750,300");
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
        host1.ethg     <--> Eth1G <--> switch6.ethg++;
        host2.ethg     <--> Eth1G <--> switch5.ethg++;
}

//
// Extends SwitchNetwork with four additional switches and four additional hosts,
// creating a larger meshed topology. Used in Steps 6-8.
//
network LargeNet extends SwitchNetwork
{
    submodules:
        switch8: EthernetSwitch {
            @display("p=475,370");
        }
        switch9: EthernetSwitch {
            @display("p=475,470");
        }
        switch10: EthernetSwitch {
            @display("p=625,470");
        }
        switch11: EthernetSwitch {
            @display("p=625,370");
        }
        host3: EthernetHost {
            @display("p=700,100");
        }
        host4: EthernetHost {
            @display("p=775,470");
        }
        host5: EthernetHost {
            @display("p=325,470");
        }
        host6: EthernetHost {
            @display("p=325,370");
        }
    connections:
        switch11.ethg++ <--> Eth1G <--> switch9.ethg++;
        switch11.ethg++ <--> Eth1G <--> switch10.ethg++;
        switch11.ethg++ <--> Eth1G <--> switch5.ethg++;
        switch9.ethg++  <--> Eth1G <--> switch10.ethg++;
        switch8.ethg++  <--> Eth1G <--> switch9.ethg++;
        switch8.ethg++  <--> Eth1G <--> switch11.ethg++;
        switch8.ethg++  <--> Eth1G <--> switch5.ethg++;
        switch8.ethg++  <--> Eth1G <--> switch2.ethg++;
        switch3.ethg++  <--> Eth1G <--> host3.ethg;
        host4.ethg      <--> Eth1G <--> switch10.ethg++;
        host5.ethg      <--> Eth1G <--> switch9.ethg++;
        host6.ethg      <--> Eth1G <--> switch8.ethg++;
}
