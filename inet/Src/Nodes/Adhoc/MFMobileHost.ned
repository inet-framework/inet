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




package org.omnetpp.inet.Nodes.Adhoc;

import org.omnetpp.inet.Applications.PingApp.PingApp;
import org.omnetpp.inet.Applications.TCPApp.TCPApp;
import org.omnetpp.inet.Applications.UDPApp.UDPApp;
import org.omnetpp.inet.Base.NotificationBoard;
import org.omnetpp.inet.Mobility.BasicMobility;
import org.omnetpp.inet.Network.Contract.InterfaceTable;
import org.omnetpp.inet.Network.IPv4.RoutingTable;
import org.omnetpp.inet.NetworkInterfaces.MF80211.Nic80211;
import org.omnetpp.inet.Nodes.INET.NetworkLayer;
import org.omnetpp.inet.Transport.TCP.TCP;
import org.omnetpp.inet.Transport.UDP.UDP;


//
// Models a mobile host with a wireless (802.11b) card in ad-hoc mode.
// This model contains the Mobility Framework's 802.11 implementation, Nic80211,
// and IP, TCP and UDP protocols. The mobility model can be dynamically
// specified with the mobilityType parameter.
//
// @see MobileHost, WirelessHost
//
module MFMobileHost
{
    parameters:
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        bool IPForward = default(false);
        string routingFile = default("");
        string mobilityType = default("NullMobility");
        @display("i=device/pocketpc_s");
    gates:
        input radioIn;
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
        tcpApp[numTcpApps]: <tcpAppType> like TCPApp {
            parameters:
                @display("p=163,67");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154");
        }
        udpApp[numUdpApps]: <udpAppType> like UDPApp {
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
        mobility: <mobilityType> like BasicMobility {
            parameters:
                @display("p=149,307");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.from_appl++;
            tcpApp[i].tcpIn <-- tcp.to_appl++;
        }

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 {
            udpApp[i].to_udp --> udp.from_app++;
            udpApp[i].from_udp <-- udp.to_app++;
        }

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        radioIn --> wlan.radioIn;
        wlan.uppergateOut --> networkLayer.ifIn[0];
        wlan.uppergateIn <-- networkLayer.ifOut[0];
}

