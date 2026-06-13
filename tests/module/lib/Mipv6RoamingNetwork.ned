//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Test network for the MIPv6 two-handover (roaming) module test. It mirrors
// examples/ipv6/mipv6roaming/Mipv6RoamingNetwork.ned but uses the legacy
// Ipv6FlatNetworkConfigurator (which assigns the aaaa:: addresses the module
// tests assert on), keeping the test independent of the example (tests must
// not depend on examples or showcases).
//
// Topology: a home network and TWO foreign networks behind a common backbone
// router, so a single mobile node performs two successive handovers as it
// roams AP_Home -> AP_1 -> AP_2.
//

package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.wireless.AccessPoint;
import inet.node.mipv6.CorrespondentNode6;
import inet.node.mipv6.HomeAgent6;
import inet.node.mipv6.WirelessHost6;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

// NOTE: the ethernetline channel is declared in MIPv6TestNetwork.ned (same package).

network Mipv6RoamingNetwork
{
    parameters:
        int total_mn;
        int total_cn;
        **.mgmt.numChannels = 5;

        @display("bgb=1050,698");
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        Home_Agent: HomeAgent6 {
            parameters:
                @display("p=249,229;i=abstract/router");
        }
        R_1: Router6 {           // access router of foreign network 1 (AP_1)
            parameters:
                @display("p=566,227");
        }
        R_2_acc: Router6 {       // access router of foreign network 2 (AP_2)
            parameters:
                @display("p=850,227");
        }
        backbone: Router6 {      // backbone hub interconnecting HA, R_1, R_2_acc and the CNs
            parameters:
                @display("p=480,355");
        }
        MN[total_mn]: WirelessHost6 {
            parameters:
                @display("p=220,404");
        }
        CN[total_cn]: CorrespondentNode6 {
            @display("p=480,595,r,50");
        }
        AP_Home: AccessPoint {
            parameters:
                @display("p=249,172;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            parameters:
                @display("p=566,172;i=device/accesspoint_s");
        }
        AP_2: AccessPoint {
            parameters:
                @display("p=850,172;i=device/accesspoint_s");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=480,467;i=device/hub_s");
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
