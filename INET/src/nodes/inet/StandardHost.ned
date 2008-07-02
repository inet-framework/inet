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


package inet.nodes.inet;

import inet.applications.pingapp.PingApp;
import inet.applications.tcpapp.TCPApp;
import inet.applications.udpapp.UDPApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.util.NAMTraceWriter;


//
// \IP host with TCP, UDP layers and applications.
//
module StandardHost
{
    parameters:
        @node();
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("n/a");
        string udpAppType = default("n/a");
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        @display("i=device/pc2");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=60,310");
        }
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
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

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

