//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.nodes.ipv6;

import inet.util.NAMTraceWriter;
import inet.transport.udp.UDP;
import inet.transport.tcp.ITCP;
import inet.transport.sctp.SCTP;
import inet.networklayer.ipv6.RoutingTable6;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.ext.ExtInterface;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.applications.udpapp.IUDPApp;
import inet.applications.tcpapp.ITCPApp;
import inet.applications.sctpapp.ISCTPApp;
import inet.applications.pingapp.PingApp;


//
// IPv6 host with TCP, SCTP and UDP layers and applications.
//
module StandardHost6
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/pc2");
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numSctpApps = default(0);
        // string tcpAppType = default(""); // Obsolete
        // string udpAppType = default(""); // Obsolete
        // string sctpAppType = default(""); // Obsolete
        string tcpType = default("TCP");  // tcp implementation (e.g. ~TCP, ~TCP_lwip, ~TCP_NSC)
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the standard host. by default no wireless
        string mobilityType = default("NoMobility"); // how the node moves around. MUST be specified if the host contains a wireless interface. see: inet.mobility
        string wlanNicType = default("Ieee80211NicSTA"); // used wireless interfaces. see:  inet.linklayer.ieee80211 or other modules implementing ~IWirelessNic
        string ethNicType = default("EthernetInterfaceFullDuplex"); // used ethernet interfaces. see: ~EtherInterface (full/half duplex) or ~EtherInterfaceFUllDuplex for simplified, full duplex only
        string pppNicType = default("PPPInterface"); // used ppp interfaces. see: ~PPPInerface and ~PPPInterfaceNoQueue
        bool IPForward = default(false);             // whether to support forwarding of IP packets (i.e. routing)
        int namid = default(-1);
        xml routingTableFile;  //# TODO a reasonable default should be provided
    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=57,294");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=57,54");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=57,134");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = IPForward;
                routingTableFile = routingTableFile;
                @display("p=57,214");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=57,368");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=186,54");
        }
        tcp: <tcpType> like ITCP {
            parameters:
                @display("p=186,141");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=336,54");
        }
        udp: UDP {
            parameters:
                @display("p=336,141");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=501,54");
        }
        sctp: SCTP {
            @display("p=501,141;i=block/wheelbarrow");
        }
        pingApp: PingApp {
            parameters:
                @display("p=501,230");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=336,230;q=queue");
        }
        wlan[numRadios]: <wlanNicType> like IWirelessNic {
            parameters:
                @display("p=132,390;q=queue");
        }
        eth[sizeof(ethg)]: <ethNicType> like IWiredNic {
            parameters:
                @display("p=250,390,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <pppNicType> like IWiredNic {
            parameters:
                @display("p=369,390,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: ExtInterface {
            parameters:
                @display("p=501,390,row,90;q=txQueue;i=block/ifcard");
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

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ipv6 --> networkLayer.sctpIn;
        networkLayer.sctpOut --> sctp.from_ipv6;

        networkLayer.pingOut --> pingApp.pingv6In;
        networkLayer.pingIn <-- pingApp.pingv6Out;

        // connections to network outside
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].uppergateOut --> networkLayer.ifIn++;
            wlan[i].uppergateIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn++;
            eth[i].netwIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn++;
            ppp[i].netwIn <-- networkLayer.ifOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].netwOut --> networkLayer.ifIn++;
            ext[i].netwIn <-- networkLayer.ifOut++;
        }
}

