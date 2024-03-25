// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.tests.mrp;

import inet.node.mrp.MrpSwitch;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EthernetHost;
import inet.common.scenario.ScenarioManager;
import inet.linklayer.configurator.common.L2NetworkConfigurator;

network Mrp4
{
    @display("bgb=500,500");
    submodules:
        node0: MrpSwitch {
            @display("p=250,100");
        }
        node1: MrpSwitch {
            @display("p=350,200");
        }
        node2: MrpSwitch {
            @display("p=250,300");
        }
        node3: MrpSwitch {
            @display("p=150,200");
        }
        scenarioManager: ScenarioManager {
            @display("p=81,51;is=s");
        }
    connections:
        node0.ethg[0] <--> Eth100M <--> node1.ethg[1];
        node1.ethg[0] <--> Eth100M <--> node2.ethg[1];
        node2.ethg[0] <--> Eth100M <--> node3.ethg[1];
        node3.ethg[0] <--> Eth100M <--> node0.ethg[1];
}

network Mrp4withHosts extends Mrp4
{
    @display("bgb=573,425");
    submodules:
        host1: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=244,43");
        }
        host2: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=460,201");
        }

        host3: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,375");
        }
        host4: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=63,201");
        }
    connections:
        node0.ethg[2] <--> Eth100M <--> host1.ethg;
        node1.ethg[2] <--> Eth100M <--> host2.ethg;
        node2.ethg[2] <--> Eth100M <--> host3.ethg;
        node3.ethg[2] <--> Eth100M <--> host4.ethg;
}

network Mrp8 extends Mrp4
{
    @display("bgb=1000,500");
    submodules:
        node4: MrpSwitch {
            @display("p=690.25,99.145");
        }
        node5: MrpSwitch {
            @display("p=837.08496,199.545");
        }
        node6: MrpSwitch {
            @display("p=690.25,299.94498");
        }
        node7: MrpSwitch {
            @display("p=557.22,199.545");
        }
        host1: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=50,201");
        }
        host2: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=459.33,251");
        }  
        host3: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=948.77997,200.79999");
        }
        host4: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=459.33,159.385");
        }
    connections:
        node4.ethg[0] <--> Eth100M <--> node5.ethg[1];
        node5.ethg[0] <--> Eth100M <--> node6.ethg[1];
        node6.ethg[0] <--> Eth100M <--> node7.ethg[1];
        node7.ethg[0] <--> Eth100M <--> node4.ethg[1];
        node0.ethg[2] <--> Eth100M <--> node4.ethg[2];
        node2.ethg[2] <--> Eth100M <--> node6.ethg[2];
        host1.ethg <--> Eth100M <--> node3.ethg[2];
        host2.ethg <--> Eth100M <--> node1.ethg[2];
        host3.ethg <--> Eth100M <--> node5.ethg[2];
        host4.ethg <--> Eth100M <--> node7.ethg[2];
}

network Mrp50
{
    @display("bgb=1154,954");
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=95,48;is=s");
        }
        nodeO[25]: MrpSwitch {
            @display("p=,,ri,360,360");
            gates:
                ethg[3];
        }
        nodeI[25]: MrpSwitch {
            @display("p=,,ri,275,275");
            gates:
                ethg[3];
        }
        hostI[25]: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=,,ri,200,200");
        }
        hostO[25]: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=,,ri,435,435");
        }
    connections:
        for i=0..23 {
            nodeO[i].ethg[0] <--> Eth100M <--> nodeI[i].ethg[1];
            nodeI[i].ethg[0] <--> Eth100M <--> nodeO[i+1].ethg[1];
        }
        nodeO[24].ethg[0] <--> Eth100M <--> nodeI[24].ethg[1];
        nodeI[24].ethg[0] <--> Eth100M <--> nodeO[0].ethg[1];
        for i=0..24 {
                hostO[i].ethg <--> Eth100M <--> nodeO[i].ethg[2];
                hostI[i].ethg <--> Eth100M <--> nodeI[i].ethg[2];
        }
}
