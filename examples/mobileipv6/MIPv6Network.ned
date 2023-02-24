// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute
// University of Dortmund, Germany
// faqir.yousaf@uni-dortmund.de
//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//


package inet.examples.mobileipv6;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.wireless.AccessPoint;
import inet.node.xmipv6.CorrespondentNode6;
import inet.node.xmipv6.HomeAgent6;
import inet.node.xmipv6.WirelessHost6;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


channel fiberline extends ned.DatarateChannel
{
    parameters:
        delay = 1us;
        datarate = 512Mbps;
}

channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

network mIPv6Network
{
    parameters:
        int total_mn;
        int total_cn;
        **.mgmt.numChannels = 5;

        @display("bgb=799,698");
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
        R_1: Router6 {
            parameters:
                @display("p=566,227");
        }
        R_2: Router6 {
            parameters:
                @display("p=406,355");
        }
        MN[total_mn]: WirelessHost6 {
            parameters:
                @display("p=220,404");
        }
        CN[total_cn]: CorrespondentNode6 {
            @display("p=405,595,r,50");
        }
        AP_Home: AccessPoint {
            parameters:
                @display("p=249,172;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            parameters:
                @display("p=566,172;i=device/accesspoint_s");
        }
        switch: EthernetSwitch {
            parameters:
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

