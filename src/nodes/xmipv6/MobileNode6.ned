//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund
// faqir.yousaf@uni-dortmund.de
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

// Same as StandardHost6.ned except with the new parameter isMobileNode = true/false
// & isHomeAgent = true/false
// Also replaced the NetworkLayer6 with MobileIPLayer6 (which is the same as Network Layer6 except with a MIP engine)

package inet.nodes.xmipv6;

import inet.transport.udp.UDP;
import inet.transport.tcp_inet.TCP;
import inet.nodes.ipv6.NetworkLayer6;
import inet.networklayer.xmipv6.BindingUpdateList;
import inet.networklayer.ipv6.RoutingTable6;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.ppp.PPPInterface;
import inet.linklayer.ethernet.EthernetInterface;
import inet.base.NotificationBoard;
import inet.applications.pingapp.PingApp;
import inet.applications.IUDPApp;
import inet.applications.ITCPApp;


//
// IPv6 host with TCP, UDP layers and applications and supports MIPv6.
//
module MobileNode6___UNUSED
{
    parameters:
        @node();
        int numTcpApps = default(0);
        int numUdpApps = default(0);
    gates:
        input in[];
        output out[];
        input ethIn[];
        output ethOut[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150;i=block/table");
        }
        buList: BindingUpdateList {
            parameters:
                @display("p=63,292;i=old/harddisk");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = false;
                //isProxy = false;
                @display("p=60,230;i=block/table");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=163,67;i=block/app");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154;i=block/wheelbarrow");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("i=block/app;p=272,67");
        }
        udp: UDP {
            parameters:
                @display("p=272,154;i=block/transport");
        }
        pingApp: PingApp {
            parameters:
                @display("i=block/app;p=343,200");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                xMIPv6Support = true;
                mipv6support.isMobileNode = true; //is Mobile Node
                mipv6support.isHomeAgent = false; //is Home Agent
                @display("p=248,247;q=queue");
            gates:
                ifIn[sizeof(out)+sizeof(ethOut)];
                ifOut[sizeof(out)+sizeof(ethOut)];
        }
        ppp[sizeof(out)]: PPPInterface {
            parameters:
                @display("p=156,350,row,90;q=txQueue;i=block/ifcard");
        }
        eth[sizeof(ethOut)]: EthernetInterface {
            parameters:
                @display("p=278,350,row,90;q=txQueue;i=block/ifcard");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipv6Out --> networkLayer.tcpIn;
        tcp.ipv6In <-- networkLayer.tcpOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipv6Out --> networkLayer.udpIn;
        udp.ipv6In <-- networkLayer.udpOut;

        networkLayer.pingOut --> pingApp.pingv6In;
        networkLayer.pingIn <-- pingApp.pingv6Out;

        // connections to network outside
        for i=0..sizeof(out)-1 {
            in[i] --> ppp[i].phys$i;
            out[i] <-- ppp[i].phys$o;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethOut)-1 {
            ethIn[i] --> eth[i].phys$i;
            ethOut[i] <-- eth[i].phys$o;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(out)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(out)+i];
        }
}

