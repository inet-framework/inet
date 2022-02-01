//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.qos;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network QosThroughput
{
    parameters:
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        ap: AccessPoint {
            parameters:
                @display("p=400,200;r=,,#707070");
        }
        srvHost: WirelessHost {
            parameters:
                @display("r=,,#707070;p=550,100");
        }
        cliHost: WirelessHost {
            parameters:
                @display("r=,,#707070;p=250,100");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
}

