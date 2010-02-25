//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//               2004 Andras Varga
//               2009 Thomas Reschka
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.nodes.inet;

import inet.applications.pingapp.PingApp;
import inet.applications.tcpapp.TCPApp;
import inet.applications.udpapp.UDPApp;
import inet.applications.sctpapp.SCTPApp;   //I.R.
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterfaceWithULThruputMeter;
import inet.linklayer.ext.ExtInterface; //I.R.
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.transport.sctp.SCTP;    //I.R.
import inet.util.TCPDump;   //I.R.
import inet.util.NAMTraceWriter;


//
// \IP host with TCP, UDP, SCTP layers and applications AND PPPInterfaceWithULThruputMeter.
//
module StandardHostWithULThruputMeter
{
    parameters:
        @node();
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numSctpApps = default(0);   //I.R.
        string tcpAppType = default("n/a");
        string udpAppType = default("n/a");
        string sctpAppType = default("n/a");    //I.R.
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        int numExtInterfaces = default(0);
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
                @display("p=163,54");
        }
        tcp: TCP {
            parameters:
                @display("p=163,141");
        }
        udpApp[numUdpApps]: <udpAppType> like UDPApp {
            parameters:
                @display("p=290,54");
        }
        udp: UDP {
            parameters:
                @display("p=290,141");
        }
        sctpApp[numSctpApps]: <sctpAppType> like SCTPApp {  //I.R.
            parameters:
                @display("p=416,54");
        }
        sctp: SCTP {    //I.R.
            @display("p=416,141;i=block/wheelbarrow");
        }
        pingApp: PingApp {
            parameters:
                @display("p=488,109");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=290,230;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];   //I.R.
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
        }
        ppp[sizeof(pppg)]: PPPInterfaceWithULThruputMeter {
            parameters:
                @display("p=163,390,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=290,390,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: ExtInterface {   //I.R.
            parameters:
                @display("p=416,390,row,90;q=txQueue;i=block/ifcard");
        }
        tcpdump: TCPDump {  //I.R.
            parameters:
                @display("p=290,302;i=abstract/cache_s");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                in2[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                out2[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
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

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.sctpIn;
        networkLayer.sctpOut --> sctp.from_ip;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> tcpdump.ifIn[i]; //I.R.
            tcpdump.out2[i] --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- tcpdump.ifOut[i];
            tcpdump.in2[i] <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> tcpdump.ifIn[sizeof(pppg)];  //I.R.
            tcpdump.out2[sizeof(pppg)] --> networkLayer.ifIn[sizeof(pppg)];
            eth[i].netwIn <-- tcpdump.ifOut[sizeof(pppg)];
            tcpdump.in2[sizeof(pppg)] <-- networkLayer.ifOut[sizeof(pppg)];
        }

        for i=0..sizeof(extOut)-1 {
            ext[i].netwOut --> tcpdump.ifIn[sizeof(pppg)+sizeof(ethg)+i];
            tcpdump.out2[sizeof(pppg)+sizeof(ethg)+i] --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)+i];
            ext[i].netwIn <-- tcpdump.ifOut[sizeof(pppg)+sizeof(ethg)+i];
            tcpdump.in2[sizeof(pppg)+sizeof(ethg)+i] <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)+i];
        }
}
