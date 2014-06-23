//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.examples.wireless.synchronized;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.Ieee80211RadioMedium;


network Synchronized
{
    parameters:
        int nodeCount;
        **.bitrate = 11Mbps;
        **.arpType = "GlobalARP";
        @display("bgl=3");
    submodules:
        radioMedium: Ieee80211RadioMedium;
        node[nodeCount]: AdhocHost {
            parameters:
                numUdpApps = 1;
                udpApp[0].typename = "UDPBasicApp";
                udpApp[0].destAddresses = "224.0.0.1";
                udpApp[0].localPort = 1;
                udpApp[0].destPort = 1;
                udpApp[0].messageLength = 100B;
                udpApp[0].joinLocalMulticastGroups = true;
        }
        configurator: IPv4NetworkConfigurator;
}

