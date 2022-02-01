//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.qos;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;


network QosShowcase
{
    parameters:
        @display("b=297,203;bgb=511,302");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62,143");
        }
        ap: AccessPoint {
            parameters:
                @display("p=300,200;r=,,#707070");
        }
        server: WirelessHost {
            parameters:
                @display("p=400,100;r=,,#707070");
        }
        client: WirelessHost {
            parameters:
                @display("p=200,100;r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=61,46");
        }
        visualizer: IntegratedVisualizer {
            @display("p=61,236");
        }
}

