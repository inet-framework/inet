//
//  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package inet.examples.hostautoconf;

import inet.applications.udpapp.UDPBasicApp;
import inet.base.NotificationBoard;
import inet.linklayer.mf80211.Nic80211;
import inet.mobility.RandomWPMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.udp.UDP;
import inet.networklayer.autorouting.HostAutoConfigurator;

//
// Wireless-enabled Host
//
module Host
{
    parameters:

        @display("bgb=424,541");
    gates:
        input radioIn;

    submodules:
        //comm_rad: Display;
        //    display: "p=140,468;i=block/app2";
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=140,462;i=block/control");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=140,326;i=block/table");
        }
        mobility: RandomWPMobility {
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
        udpapp: UDPBasicApp {
            parameters:
                @display("i=block/app;p=384,47");
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
        wlan: Nic80211 {
            parameters:
                @display("p=304,461;q=queue;i=block/ifcard");
        }
        hostAutoConfigurator: HostAutoConfigurator {
            @display("p=171,335");
        }
    connections allowunconnected:
        udp.appOut++ --> udpapp.udpIn;
        udp.appIn++ <-- udpapp.udpOut;

        networkLayer.udpOut --> udp.ipIn;
        networkLayer.udpIn <-- udp.ipOut;

        wlan.uppergateOut --> networkLayer.ifIn[0];
        wlan.uppergateIn <-- networkLayer.ifOut[0];

        radioIn --> wlan.radioIn;

}

