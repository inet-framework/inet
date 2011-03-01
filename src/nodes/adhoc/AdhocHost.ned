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

package inet.nodes.adhoc;

import inet.transport.udp.UDP;
import inet.transport.tcp.TCP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWirelessNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.applications.udpapp.IUDPApp;
import inet.applications.tcpapp.ITCPApp;
import inet.applications.pingapp.PingApp;


//
// A wireless host containing routing, mobility and battery components.
// Supports only IPv4 protocol, TCP and UDP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// The applications, the mobility type, the wireless interface can be
// configured via parameters.
//
module AdhocHost
{
    @node();
    int numTcpApps = default(0);
    int numUdpApps = default(0);
    string tcpAppType = default("");
    string udpAppType = default("");
    string routingFile = default("");
    string mobilityType = default("NullMobility"); // how the node moves around. see: inet.mobility
    string wlanNicType = default("Ieee80211NicAdhoc"); // used wireless interfaces. see:  inet.linklayer.ieee80211 or other modules implementing IWirelessNic
    int numRadios = default(1);
    @display("i=device/cellphone;bgb=525,394");

    gates:
        input radioIn[numRadios] @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,67");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,154");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,247");
        }
        mobility: <mobilityType> like IMobility {
            @display("p=60,331");
        }
        tcpApp[numTcpApps]: <tcpAppType> like ITCPApp {
            @display("p=198,67");
        }
        tcp: TCP {
            @display("p=198,154");
        }
        udpApp[numUdpApps]: <udpAppType> like IUDPApp {
            @display("p=346,67");
        }
        udp: UDP {
            @display("p=346,154");
        }
        pingApp: PingApp {
            @display("p=451,154");
        }
        networkLayer: NetworkLayer {
            proxyARP = false;
            @display("p=279,247;q=queue");
        }
        wlan[numRadios]: <wlanNicType> like IWirelessNic {
            @display("p=279,331,row,90;q=queue");
        }
    connections allowunconnected:
        for i=0..sizeof(tcpApp)-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        for i=0..sizeof(udpApp)-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].uppergateOut --> networkLayer.ifIn++;
            wlan[i].uppergateIn <-- networkLayer.ifOut++;
        }
}
