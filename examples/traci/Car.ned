//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package inet.examples.traci;

import inet.networklayer.autorouting.HostAutoConfigurator;
import inet.transport.udp.UDP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.traci.TraCIMobility;
import inet.linklayer.ieee80211.Ieee80211NicAdhoc;
import inet.base.NotificationBoard;
import inet.applications.traci.TraCIDemo;

module Car
{
    parameters:

        @display("bgb=424,541");
    gates:
        input radioIn;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=140,462;i=block/control");
        }
        ac_wlan: HostAutoConfigurator {
            @display("p=296,402");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=140,326;i=block/table");
        }
        mobility: TraCIMobility {
            parameters:
                @display("p=60,459;i=block/cogwheel");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "";
                routingFile = "";
                @display("p=60,326;i=block/table");
        }
        udp: UDP {
            parameters:
                @display("p=384,146;i=block/transport");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=304,327;i=block/fork;q=queue");
            gates:
                ifIn[1];
                ifOut[1];
        }
        wlan: Ieee80211NicAdhoc {
            parameters:
                @display("p=304,461;q=queue;i=block/ifcard");
        }
        app: TraCIDemo {
            parameters:
                @display("p=384,46;i=block/app");
        }
    connections allowunconnected:
        udp.appOut++ --> app.udp$i;
        udp.appIn++ <-- app.udp$o;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        wlan.uppergateOut --> networkLayer.ifIn[0];
        wlan.uppergateIn <-- networkLayer.ifOut[0];

        radioIn --> wlan.radioIn;
}

