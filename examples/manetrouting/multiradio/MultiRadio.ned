//
// Copyright (C) 2010 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.manetrouting.multiradio;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.dymo.DymoRouter;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


//
// Models a network with several hosts. Each host may contain one or
// more radios. Nodes are using adhoc routing to pass information.
//
network MultiRadio
{
    parameters:
        int numHosts;
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            config = xml("<config><interface hosts='*' address='10.0.0.x' netmask='255.255.255.0'/></config>");
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
            @display("p=100,100;is=s");
        }
        mobileHost[numHosts]: DymoRouter {
            @display("p=250,150;r=,,#000040");
        }
    connections allowunconnected:
}
