//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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





package org.omnetpp.inet.Nodes.IPv6;

import org.omnetpp.inet.Applications.PingApp.PingApp;
import org.omnetpp.inet.Applications.TCPApp.TCPApp;
import org.omnetpp.inet.Applications.UDPApp.UDPApp;
import org.omnetpp.inet.Base.NotificationBoard;
import org.omnetpp.inet.Network.Contract.InterfaceTable;
import org.omnetpp.inet.Network.IPv6.RoutingTable6;
import org.omnetpp.inet.NetworkInterfaces.Ethernet.EthernetInterface;
import org.omnetpp.inet.NetworkInterfaces.PPP.PPPInterface;
import org.omnetpp.inet.Transport.TCP.TCP;
import org.omnetpp.inet.Transport.UDP.UDP;


//
// \IPv6 host with TCP, UDP layers and applications.
//
module StandardHost6
{
    parameters:
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        @display("i=device/laptop");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = false;
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
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=248,247;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=205,350,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=240,350,row,90;q=txQueue");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.from_appl++;
            tcpApp[i].tcpIn <-- tcp.to_appl++;
        }

        tcp.to_ipv6 --> networkLayer.TCPIn;
        tcp.from_ipv6 <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 {
            udpApp[i].to_udp --> udp.from_app++;
            udpApp[i].from_udp <-- udp.to_app++;
        }

        udp.to_ipv6 --> networkLayer.UDPIn;
        udp.from_ipv6 <-- networkLayer.UDPOut;

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
}

