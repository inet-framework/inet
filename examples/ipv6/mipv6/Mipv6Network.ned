//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2007 Faqir Zarrar Yousaf, Communication Networks Institute, University of Dortmund, Germany (faqir.yousaf@uni-dortmund.de)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ipv6.mipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.wireless.AccessPoint;
import inet.node.mipv6.CorrespondentNode6;
import inet.node.mipv6.HomeAgent6;
import inet.node.mipv6.WirelessHost6;
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

network Mipv6Network
{
    parameters:
        int total_mn;
        int total_cn;
        **.mgmt.numChannels = 5;
        **.ipv6.configurator.networkConfiguratorModule = "ipv6Configurator";
        @display("bgb=800,700");

    submodules:
        ipv6Configurator: Ipv6NetworkConfigurator {
            parameters:
                // The mobile node must autoconfigure its home/care-of addresses (SLAAC),
                // so the configurator addresses only routers and lets hosts use SLAAC.
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
        R_1: Router6 {
            parameters:
                @display("p=600,300");
        }
        R_2: Router6 {
            parameters:
                @display("p=450,400");
        }
        MN[total_mn]: WirelessHost6 {
            parameters:
                @display("p=300,150");
        }
        CN[total_cn]: CorrespondentNode6 {
            @display("p=450,600,r,50");
        }
        AP_Home: AccessPoint {
            parameters:
                @display("p=300,200;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            parameters:
                @display("p=600,200;i=device/accesspoint_s");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=450,500;i=device/hub_s");
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
