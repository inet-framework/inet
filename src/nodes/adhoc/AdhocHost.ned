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


import inet.transport.IUDP;
import inet.transport.ITCP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWirelessNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.applications.pingapp.PingApp;
import inet.applications.IUDPApp;
import inet.applications.ITCPApp;


//
// A wireless host containing routing, mobility and battery components.
// Supports only IPv4 protocol, TCP and UDP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// - By default contains a single wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.type-name parameter. see:  inet.linklayer.ieee80211 or other 
//   modules implementing ~IWirelessNic
// - Node mobility can be set using **.mobility.type-name
//   see: inet.mobility and ~IMobility
//
module AdhocHost
{
    @node();
    @display("i=device/cellphone;bgb=525,394");
    int numTcpApps = default(0);
    int numUdpApps = default(0);
    // string tcpAppType = default(""); // Obsolete!
    // string udpAppType = default(""); // Obsolete!
    string tcpType = default(haveClass("TCP")?"TCP":haveClass("TCP_lwip")?"TCP_lwip":haveClass("TCP_NSC")?"TCP_NSC":"TCP_None");  // tcp implementation (e.g. ~TCP, ~TCP_lwip, ~TCP_NSC) or ~TCPSpoof
    int numRadios = default(1);
    string wlanNicType = default("Ieee80211NicAdhoc");
    string routingFile = default("");
    bool IPForward = default(true);

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
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,247");
        }
        mobility: <default("NullMobility")> like IMobility {
            @display("p=60,331");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            @display("p=198,67");
        }
        tcp: <tcpType> like ITCP {
            @display("p=198,154");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            @display("p=346,67");
        }
        udp: <default(haveClass("UDP")?"UDP":"UDP_None")> like IUDP {
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
