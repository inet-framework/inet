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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.nodes.adhoc;

import inet.transport.udp.UDP;
import inet.transport.tcp.TCP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.mf80211.Nic80211;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.applications.udpapp.IUDPApp;
import inet.applications.tcpapp.ITCPApp;
import inet.applications.pingapp.PingApp;


//
// Models a mobile host with a wireless (802.11b) card in ad-hoc mode.
// This model contains the Mobility Framework's 802.11 implementation ~Nic80211,
// and IP, TCP and UDP protocols. The mobility model can be dynamically
// specified with the mobilityType parameter.
//
//(deprecated - use AdhocHost instead with Nic80211 as the NIC parameter)
//
// @see ~MobileHost, ~WirelessHost
//
module MFMobileHost
{
    parameters:
        @node();
        @labels(node);
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        // string tcpAppType = default(""); // Obsolete
        // string udpAppType = default(""); // Obsolete
        bool IPForward = default(false);
        string routingFile = default("");
        string mobilityType = default("NullMobility");
        @display("i=device/pocketpc_s");
    gates:
        input radioIn @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,230");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=163,67");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=272,67");
        }
        udp: UDP {
            parameters:
                @display("p=272,154");
        }
        pingApp: PingApp {
            parameters:
                @display("p=343,200");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=248,247;q=queue");
            gates:
                ifIn[1];
                ifOut[1];
        }
        wlan: Nic80211 {
            parameters:
                @display("p=248,349;q=queue");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=149,307");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        radioIn --> wlan.radioIn;
        wlan.uppergateOut --> networkLayer.ifIn[0];
        wlan.uppergateIn <-- networkLayer.ifOut[0];
}

