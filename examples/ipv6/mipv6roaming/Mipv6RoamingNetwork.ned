//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ipv6.mipv6roaming;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.wireless.AccessPoint;
import inet.node.mipv6.CorrespondentNode6;
import inet.node.mipv6.HomeAgent6;
import inet.node.mipv6.WirelessHost6;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

//
// A model with three access networks, where the mobile node performs two
// successive handovers: its home network (AP_Home, behind the home agent) and two
// distinct foreign networks (AP_1 behind R_1, AP_2 behind R_2_acc). As the mobile node
// roams home -> AP_1 -> AP_2 it forms a new care-of address in each foreign network and
// re-registers it with the home agent.
//
network Mipv6RoamingNetwork
{
    parameters:
        int total_mn;
        int total_cn;
        **.mgmt.numChannels = 5;
        **.ipv6.configurator.networkConfiguratorModule = "ipv6Configurator";
        @display("bgb=1100,700");

    submodules:
        ipv6Configurator: Ipv6NetworkConfigurator {
            parameters:
                // mobile nodes autoconfigure their home/care-of addresses via SLAAC
                assignAddressesToHosts = false;
                @display("p=100,150;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250;is=s");
        }
        Home_Agent: HomeAgent6 {
            parameters:
                @display("p=300,300;i=abstract/router");
        }
        R_1: Router6 {           // access router of foreign network 1 (AP_1)
            parameters:
                @display("p=600,300");
        }
        R_2_acc: Router6 {       // access router of foreign network 2 (AP_2)
            parameters:
                @display("p=900,300");
        }
        backbone: Router6 {      // backbone hub interconnecting HA, R_1, R_2_acc and the CNs
            parameters:
                @display("p=600,400");
        }
        MN[total_mn]: WirelessHost6 {
            parameters:
                @display("p=300,150");
        }
        CN[total_cn]: CorrespondentNode6 {
            @display("p=600,600,r,50");
        }
        AP_Home: AccessPoint {
            parameters:
                @display("p=300,200;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            parameters:
                @display("p=600,200;i=device/accesspoint_s");
        }
        AP_2: AccessPoint {
            parameters:
                @display("p=900,200;i=device/accesspoint_s");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=600,500;i=device/hub_s");
        }

    connections allowunconnected:
        Home_Agent.ethg++ <--> ethernetline <--> backbone.ethg++;
        R_1.ethg++ <--> ethernetline <--> backbone.ethg++;
        R_2_acc.ethg++ <--> ethernetline <--> backbone.ethg++;

        for i=0..total_cn-1 {
            CN[i].ethg++ <--> ethernetline <--> switch.ethg++;
        }
        switch.ethg++ <--> ethernetline <--> backbone.ethg++;

        AP_Home.ethg++ <--> ethernetline <--> Home_Agent.ethg++;
        AP_1.ethg++ <--> ethernetline <--> R_1.ethg++;
        AP_2.ethg++ <--> ethernetline <--> R_2_acc.ethg++;
}
