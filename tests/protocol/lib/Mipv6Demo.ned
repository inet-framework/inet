//
// Protocol Test Framework for INET -- Mobile IPv6 (RFC 6275) demo network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
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
// A minimal Mobile IPv6 handover/route-optimization topology (one mobile node, one
// correspondent node, one home agent), modelled on INET's proven Mipv6Network so the
// handover + return-routability exchange fire deterministically. A ProtocolTester observes
// the resulting Mobility Header message sequence (Binding Update / Acknowledgement, the
// HoTI/CoTI/HoT/CoT return-routability messages).
//
// The mobile node MN[0] (a WirelessHost6) roams between AP_Home (on the home link, behind
// the Home Agent) and AP_1 (a foreign link behind R_1); when away it registers its care-of
// address with the Home Agent and route-optimizes with the correspondent node CN[0].
//
network Mipv6Demo
{
    parameters:
        int total_mn;
        int total_cn;
        **.mgmt.numChannels = 5;
        @display("bgb=799,698");
    submodules:
        tester: ProtocolTester {
            @display("p=100,50;is=s");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
        ipv6Configurator: Ipv6NetworkConfigurator {
            @display("p=100,150;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        Home_Agent: HomeAgent6 {
            @display("p=249,229;i=abstract/router");
        }
        R_1: Router6 {
            @display("p=566,227");
        }
        R_2: Router6 {
            @display("p=406,355");
        }
        MN[total_mn]: WirelessHost6 {
            @display("p=220,404");
        }
        CN[total_cn]: CorrespondentNode6 {
            @display("p=405,595,r,50");
        }
        AP_Home: AccessPoint {
            @display("p=249,172;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            @display("p=566,172;i=device/accesspoint_s");
        }
        switch: EthernetSwitch {
            @display("p=405,467;i=device/hub_s");
        }
    connections allowunconnected:
        R_1.ethg++ <--> ethernetline <--> R_2.ethg++;
        Home_Agent.ethg++ <--> ethernetline <--> R_2.ethg++;
        for i=0..total_cn-1 {
            CN[i].ethg++ <--> ethernetline <--> switch.ethg++;
        }
        switch.ethg++ <--> ethernetline <--> R_2.ethg++;
        AP_Home.ethg++ <--> ethernetline <--> Home_Agent.ethg++;
        AP_1.ethg++ <--> ethernetline <--> R_1.ethg++;
}
