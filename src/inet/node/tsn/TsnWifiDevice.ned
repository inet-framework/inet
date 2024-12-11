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

package inet.node.tsn;
import inet.node.base.NodeBase;
import inet.common.MessageDispatcher;

//App Laayer
import inet.applications.contract.IApp;

import inet.transportlayer.contract.ISctp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;


import inet.networklayer.contract.INetworkLayer;

import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITunnelInterface;
import inet.linklayer.contract.IVirtualInterface;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8021q.IIeee8021qLayer;
import inet.linklayer.ieee8021r.IIeee8021rLayer;
import inet.linklayer.ieee8022.IIeee8022Llc;
import inet.networklayer.common.InterfaceTable;
import inet.node.contract.IEthernetNetworkNode;
import inet.protocolelement.contract.IProtocolLayer;
//
// TODO documentation
//
module TsnWifiDevice extends NodeBase
{
    parameters:

        @display("i=device/wifilaptop;bgb=1700.79,1323.608");
        @figure[submodules];

        ipv4.arp.proxyArpInterfaces = default(""); // proxy arp is disabled on hosts by default

        *.routingTableModule = default("^.ipv4.routingTable");

        //Appln Layer
        int numApps = default(0);
        @figure[applicationLayer](type=rectangle; pos=250,5; size=1000,137; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=1245,10; anchor=ne; text="application layer");

        //Transport layer
        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");
        bool hasSctp = default(false);
        @figure[transportLayer](type=rectangle; pos=250,158; size=1000,134; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=1245,163; anchor=ne; text="transport layer");

        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        //Linklayer
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        int numLoInterfaces = default(1);
        int numWlanInterfaces = default(1);
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        int numPppInterfaces = default(0);  // minimum number of PPP interfaces
        int numTunInterfaces = default(0);
        int numVirtInterfaces = default(0);
        string fcsMode @enum("declared","computed") = default("declared");
        pcapRecorder[*].pcapFile = default("results/" + expand("${configname}-") + fullPath() + ".pcap");
        mobility.typename = default(numWlanInterfaces > 0 ? "StationaryMobility" : "");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.fcsMode = this.fcsMode;
        wlan[*].radio.antenna.mobilityModule = default("^.^.^.mobility");
        // ethernet.registerProtocol = default(true);
        @figure[linkLayer](type=rectangle; pos=250,458; size=1000,284; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=1245,463; anchor=ne; text="link layer");
        @figure[interfaceLayer](type=rectangle; pos=250,758; size=1000,360; fillColor=#00ffff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[interfaceLayer.title](type=text; pos=1245,763; anchor=ne; text="interface layer");
        @display("bgb=1596.406,1119.968");

        @figure[networkLayer](type=rectangle; pos=250,308; size=1000,134; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=1245,313; anchor=ne; text="network layer");

        bool hasTimeSynchronization = default(false); // enable IEEE 802.1 AS time synchronization
        bool hasIngressTrafficFiltering = default(false); // enable IEEE 802.1 Qci ingress per-stream filtering
        bool hasEgressTrafficFiltering = default(false); // enable IEEE 802.1 Qci egress per-stream filtering
        bool hasEgressTrafficShaping = default(false); // enable IEEE 802.1 egress traffic shaping (credit based shaping, time aware shaping, asynchronous shaping)
        bool hasStreamRedundancy = default(false); // enable IEEE 802.1 CB frame replication and elimination
        bool hasIncomingStreams = default(false); // enable IEEE 802.1 stream decoding
        bool hasOutgoingStreams = default(false); // enable IEEE 802.1 stream identification and stream encoding
        bool hasFramePreemption = default(false); // enable IEEE 802.1 Qbu frame preemption
        bool hasCutthroughSwitching = default(false); // enable cut-through switching support
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model
        ethernet.typename = default("EthernetLayer"); // use Ethernet protocol layer outside of network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // switch to modular Ethernet interface
        eth[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");
        eth[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "Ieee8021qTimeAwareShaper" : (hasFramePreemption ? "" : "PacketQueue")); // use priority queue having multiple subqueues controlled by separate gates
        eth[*].phyLayer.typename = default(hasCutthroughSwitching ? "EthernetStreamingPhyLayer" : (hasFramePreemption ? "EthernetPreemptingPhyLayer" : "EthernetPhyLayer")); // use packet streaming when cut-through switching is enabled
//        bridging.typename = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy || hasIngressTrafficFiltering || hasEgressTrafficFiltering ? "BridgingLayer" : ""); // switch to modular bridging
//        bridging.interfaceRelay.typename = default(""); // disable frame relaying
//        bridging.streamIdentifier.typename = default(hasOutgoingStreams || hasStreamRedundancy ? "StreamIdentifierLayer" : ""); // enable stream identification when stream redundancy is enabled
//        bridging.streamIdentifier.identifier.hasSequenceNumbering = default(hasStreamRedundancy); // enable sequence numberinf if stream redundancy is enabled
//        bridging.streamRelay.typename = default(hasStreamRedundancy ? "StreamRelayLayer" : ""); // enable stream merging and stream splitting when stream redundancy is enabled
//        bridging.streamFilter.typename = default(hasIngressTrafficFiltering || hasEgressTrafficFiltering ? "StreamFilterLayer" : ""); // enable stream filtering when ingress or egress per-stream filtering is enabled
//        bridging.streamFilter.ingress.typename = default(hasIngressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // use IEEE 802.1 Qci ingress filter when ingress per-stream filtering is enabled
//        bridging.streamFilter.egress.typename = default(hasEgressTrafficFiltering ? "SimpleIeee8021qFilter" : ""); // use IEEE 802.1 Qci egress filter when egress per-stream filtering is enabled
//        bridging.streamCoder.typename = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy ? "StreamCoderLayer" : ""); // enable stream endocing/decoding when stream redundancy is enabled
        ieee8021r.typename = default(hasStreamRedundancy ? "Ieee8021rProtocol" : "");
        ieee8021q.typename = default(hasIncomingStreams || hasOutgoingStreams || hasStreamRedundancy ? "Ieee8021qProtocol" : "");
     //    
    	  //ieee8021r.typename = default(hasStreamRedundancy ? "Ieee8021rProtocol" : "");
        //ieee8021q.typename = default("Ieee8021qProtocol");
        llc.typename = default("");
    gates:
        input radioIn[numWlanInterfaces] @directIn @allowUnconnected;
        inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;
        //Custom
        inout virtual[numWlanInterfaces] @labels(HostWireless-conn) @allowUnconnected;
    submodules:
        //App Layer
        app[numApps]: <> like IApp {
            @display("p=375,75,row,150");
        }
        at: MessageDispatcher {
            @display("p=750,150;b=1000,5,,,,1");
        }
        //Transport Layer
        udp: <default(firstAvailableOrEmpty("Udp"))> like IUdp if hasUdp {
            @display("p=375,225");
        }
        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            @display("p=525,225");
        }
        sctp: <default(firstAvailableOrEmpty("Sctp"))> like ISctp if hasSctp {
            @display("p=675,225");
        }
        tn: MessageDispatcher {
            @display("p=750,300;b=1000,5,,,,1");
        }
        //Link Layer
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            @display("p=375,375;q=queue");
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            @display("p=525,375;q=queue");
        }
        generic: <default("")> like INetworkLayer if hasGn {
            @display("p=675,375;q=queue");
        }
        nl: MessageDispatcher {
            @display("p=750,450;b=1000,5,,,,1");
        }
        //LinkLayer
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=125,640;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=125,240;is=s");
        }
        llc: <default("")> like IIeee8022Llc if typename != "" {
            @display("p=375,525");
        }
        cb: MessageDispatcher {
            @display("p=750,600;b=1000,5");
        }
        bridging: <default("")> like IProtocolLayer if typename != "" {
            @display("p=750,675");
        }
        bl: MessageDispatcher {
            @display("p=750,750;b=1000,5");
        }
        ethernet: <default(sizeof(ethg) > 0 ? "EthernetEncapsulation" : "")> like IEthernetLayer if typename != "" {
            @display("p=194.188,824.17004");
        }
        ieee8021q: <default("")> like IIeee8021qLayer if typename != "" {
            @display("p=525,825");
        }
        ieee8021r: <default("")> like IIeee8021rLayer if typename != "" {
            @display("p=1088.3561,824.17004");
        }
        li: MessageDispatcher {
            @display("p=750,900;b=1000,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=749.73596,1207.908,row,150");
        }
        // TODO move wlan interfaces after eth interfaces, but it changes IP address assignment and breaks examples/inet/configurator/complex.ini
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            @display("p=462.89,1016.10004,row,150;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            @display("p=178.382,973.19806,row,150;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            @display("p=900,975,row,150;q=txQueue");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunnelInterface {
            @display("p=1110.936,1029.6481,row,150;q=txQueue");
        }
        virt[numVirtInterfaces]: <default("VirtualInterface")> like IVirtualInterface {
            @display("p=1393.186,1000.294,row,150;q=txQueue");
        }

    connections allowunconnected:
        //App Layer
        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
        //Transport layer
        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        sctp.ipOut --> tn.in++ if hasSctp;
        tn.out++ --> sctp.ipIn if hasSctp;

        tn.out++ --> ipv4.transportIn if hasIpv4;
        tn.in++ <-- ipv4.transportOut if hasIpv4;

        tn.out++ --> ipv6.transportIn if hasIpv6;
        tn.in++ <-- ipv6.transportOut if hasIpv6;

        tn.out++ --> generic.transportIn if hasGn;
        tn.in++ <-- generic.transportOut if hasGn;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;
        //Link layer
        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        generic.ifIn <-- nl.out++ if exists(generic);
        generic.ifOut --> nl.in++ if exists(generic);

        cb.out++ --> nl.in++;
        cb.in++ <-- nl.out++;

        llc.lowerLayerOut --> cb.in++ if exists(llc);
        llc.lowerLayerIn <-- cb.out++ if exists(llc);

        cb.out++ --> bridging.upperLayerIn if exists(bridging);
        bridging.upperLayerOut --> cb.in++ if exists(bridging);

        bridging.lowerLayerOut --> bl.in++ if exists(bridging);
        bl.out++ --> bridging.lowerLayerIn if exists(bridging);

        cb.out++ --> bl.in++ if !exists(bridging);
        bl.out++ --> cb.in++ if !exists(bridging);

        bl.out++ --> li.in++;
        li.out++ --> bl.in++;

        bl.out++ --> ieee8021q.upperLayerIn if exists(ieee8021q);
        ieee8021q.upperLayerOut --> bl.in++ if exists(ieee8021q);

        bl.out++ --> ieee8021r.upperLayerIn if exists(ieee8021r);
        ieee8021r.upperLayerOut --> bl.in++ if exists(ieee8021r);

        bl.out++ --> ethernet.upperLayerIn if exists(ethernet);
        ethernet.upperLayerOut --> bl.in++ if exists(ethernet);

        ieee8021q.lowerLayerOut --> li.in++ if exists(ieee8021q);
        li.out++ --> ieee8021q.lowerLayerIn if exists(ieee8021q);

        ieee8021r.lowerLayerOut --> li.in++ if exists(ieee8021r);
        li.out++ --> ieee8021r.lowerLayerIn if exists(ieee8021r);

        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
        }

        for i=0..numLoInterfaces-1 {
            li.out++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> li.in++;
        }

        for i=0..sizeof(radioIn)-1 {
            wlan[i].upperLayerOut --> li.in++;
            wlan[i].upperLayerIn <-- li.out++;
        }

        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerOut --> li.in++;
            eth[i].upperLayerIn <-- li.out++;
        }

        for i=0..sizeof(pppg)-1 {
            ppp[i].upperLayerOut --> li.in++;
            ppp[i].upperLayerIn <-- li.out++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> li.in++;
            tun[i].upperLayerIn <-- li.out++;
        }

        for i=0..numVirtInterfaces-1 {
            virt[i].upperLayerOut --> li.in++;
            virt[i].upperLayerIn <-- li.out++;
        }
}
