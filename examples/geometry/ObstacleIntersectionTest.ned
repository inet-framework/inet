//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.geometry;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

network ObstacleIntersectionTest
{
    parameters:
        int numHosts;
        @display("bgb=2000,1000");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("is=s;p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("is=s;p=100,300");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
                @display("is=s;p=100,100");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
}
