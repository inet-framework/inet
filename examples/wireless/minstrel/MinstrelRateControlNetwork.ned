//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.wireless.minstrel;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;

network MinstrelRateControlNetwork
{
    parameters:
        @display("bgb=640,300");
    submodules:
        configurator: Ipv4NetworkConfigurator { @display("p=70,50"); }
        radioMedium: Ieee80211DimensionalRadioMedium { @display("p=230,50"); }
        source: WirelessHost { @display("p=100,180"); }
        ap: AccessPoint { @display("p=320,180"); }
        sink: StandardHost { @display("p=530,180"); }
    connections:
        ap.ethg++ <--> Eth100M <--> sink.ethg++;
}
