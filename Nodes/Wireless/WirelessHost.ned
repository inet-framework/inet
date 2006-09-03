//
// Copyright (C) 2006 Andras Varga
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
    "PPPInterface",
    "EthernetInterface",
    "Ieee80211Nic";


//
// Models a host with one wireless (802.11b) card in infrastructure mode.
// This module is basically a StandardHost with an Ieee80211NicSTA added.
// It should be used in conjunction with WirelessAP, or any other AP model
// which contains Ieee80211NicAP.
//
// @see WirelessAP, WirelessAPSimplified, WirelessAPWithEth, WirelessAPWithEthSimplified
// @see WirelessHost, WirelessHostSimplified
// @see MobileHost, MFMobileHost
//
module WirelessHost
    parameters:
        numTcpApps: numeric const,
        numUdpApps: numeric const,
        tcpAppType: string,
        udpAppType: string,
        IPForward: bool,
        routingFile: string,
        mobilityType: string;
    gates:
        in: in[];
        out: out[];
        in: ethIn[];
        out: ethOut[];
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
                ifIn[sizeof(out)+sizeof(ethOut)+1],
                ifOut[sizeof(out)+sizeof(ethOut)+1];
            display: "p=248,247;i=block/fork;q=queue";
        ppp: PPPInterface[sizeof(out)];
            display: "p=205,350,row,90;q=txQueue;i=block/ifcard";
        eth: EthernetInterface[sizeof(ethOut)];
            display: "p=240,350,row,90;q=txQueue;i=block/ifcard";
        wlan: Ieee80211NicSTA;
            display: "p=120,350;q=queue;i=block/ifcard";
        mobility: mobilityType like BasicMobility;
            display: "p=58,301;i=block/cogwheel_s";
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
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;

        for i=0..sizeof(ethOut)-1 do
            ethIn[i] --> eth[i].physIn;
            ethOut[i] <-- eth[i].physOut;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(out)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(out)+i];
        endfor;

        radioIn --> wlan.radioIn;
        wlan.uppergateOut --> networkLayer.ifIn[sizeof(out)+sizeof(ethOut)];
        wlan.uppergateIn <-- networkLayer.ifOut[sizeof(out)+sizeof(ethOut)];
endmodule

