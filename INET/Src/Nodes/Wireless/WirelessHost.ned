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




package org.omnetpp.inet.Nodes.Wireless;

import org.omnetpp.inet.Applications.PingApp.PingApp;
import org.omnetpp.inet.Applications.TCPApp.TCPApp;
import org.omnetpp.inet.Applications.UDPApp.UDPApp;
import org.omnetpp.inet.Base.NotificationBoard;
import org.omnetpp.inet.Mobility.BasicMobility;
import org.omnetpp.inet.Network.Contract.InterfaceTable;
import org.omnetpp.inet.Network.IPv4.RoutingTable;
import org.omnetpp.inet.NetworkInterfaces.Ethernet.EthernetInterface;
import org.omnetpp.inet.NetworkInterfaces.Ieee80211.Ieee80211NicSTA;
import org.omnetpp.inet.NetworkInterfaces.PPP.PPPInterface;
import org.omnetpp.inet.Nodes.INET.NetworkLayer;
import org.omnetpp.inet.Transport.TCP.TCP;
import org.omnetpp.inet.Transport.UDP.UDP;


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
{
    parameters:
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        bool IPForward = default(false);
        string routingFile = default("");
        string mobilityType = default("NullMobility");
        @display("i=device/wifilaptop");
    gates:
        inout pppg[];
        inout ethg[];
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
                ifIn[sizeof(pppg)+sizeof(ethg)+1];
                ifOut[sizeof(pppg)+sizeof(ethg)+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=205,350,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=240,350,row,90;q=txQueue");
        }
        wlan: Ieee80211NicSTA {
            parameters:
                @display("p=120,350;q=queue");
        }
        mobility: <mobilityType> like BasicMobility {
            parameters:
                @display("p=58,301");
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
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }

        radioIn --> wlan.radioIn;
        wlan.uppergateOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)];
        wlan.uppergateIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)];
}

