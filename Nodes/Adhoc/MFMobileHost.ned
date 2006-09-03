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


import
    "NotificationBoard",
    "InterfaceTable",
    "RoutingTable",
    "TCPApp.ned",
    "TCP.ned",
    "UDP",
    "UDPApp",
    "NetworkLayer",
    "PingApp",
    "Nic80211",
    "BasicMobility";


//
// Models a mobile host with a wireless (802.11b) card in ad-hoc mode.
// This model contains the Mobility Framework's 802.11 implementation, Nic80211,
// and IP, TCP and UDP protocols. The mobility model can be dynamically
// specified with the mobilityType parameter.
//
// @see MobileHost, WirelessHost
//
module MFMobileHost
    parameters:
        numTcpApps: numeric const,
        numUdpApps: numeric const,
        tcpAppType: string,
        udpAppType: string,
        IPForward: bool,
        routingFile: string,
        mobilityType: string;
    gates:
        in: radioIn;
    submodules:
        notificationBoard: NotificationBoard;
            display: "p=60,70;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=60,150;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routerId = "",
                routingFile = routingFile;
            display: "p=60,230;i=block/table";
        tcpApp: tcpAppType[numTcpApps] like TCPApp;
            display: "p=163,67;i=block/app";
        tcp: TCP;
            display: "p=163,154;i=block/wheelbarrow";
        udpApp: udpAppType[numUdpApps] like UDPApp;
            display: "i=block/app;p=272,67";
        udp: UDP;
            display: "p=272,154;i=block/transport";
        pingApp: PingApp;
            display: "i=block/app;p=343,200";
        networkLayer: NetworkLayer;
            parameters:
                proxyARP = false;
            gatesizes:
                ifIn[1],
                ifOut[1];
            display: "p=248,247;i=block/fork;q=queue";
        wlan: Nic80211;
            display: "p=248,349;q=queue;i=block/ifcard";
        mobility: mobilityType like BasicMobility;
            display: "p=149,307;i=block/cogwheel";
    connections nocheck:
        for i=0..numTcpApps-1 do
            tcpApp[i].tcpOut --> tcp.from_appl++;
            tcpApp[i].tcpIn <-- tcp.to_appl++;
        endfor;

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 do
            udpApp[i].to_udp --> udp.from_app++;
            udpApp[i].from_udp <-- udp.to_app++;
        endfor;

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        radioIn --> wlan.radioIn;
        wlan.uppergateOut --> networkLayer.ifIn[0];
        wlan.uppergateIn <-- networkLayer.ifOut[0];
endmodule

