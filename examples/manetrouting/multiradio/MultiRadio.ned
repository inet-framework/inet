//
// Copyright (C) 2010 Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
package inet.examples.manetrouting.multiradio;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.dymo.DymoRouter;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


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
