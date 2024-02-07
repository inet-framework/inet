//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.base;

import inet.common.MessageDispatcher;
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
import inet.common.MessageDispatcher;
import inet.networklayer.contract.INetworkLayer;


module NetworkTranslator extends NodeBase like IEthernetNetworkNode
{
    parameters:


        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        int numLoInterfaces = default(1);
        int numWlanInterfaces = default(0);
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
        ethernet.registerProtocol = default(true);

        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        @figure[networkLayer](type=rectangle; pos=250,308; size=1000,134; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=1245,313; anchor=ne; text="network layer");



        @figure[linkLayer](type=rectangle; pos=250,458; size=1000,284; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=1245,463; anchor=ne; text="link layer");
        @figure[interfaceLayer](type=rectangle; pos=250,758; size=1000,360; fillColor=#00ffff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[interfaceLayer.title](type=text; pos=1245,763; anchor=ne; text="interface layer");
    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;
    submodules:
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
            @display("p=636.75604,659.336");
        }
        bl: MessageDispatcher {
            @display("p=750,750;b=1000,5");
        }
        ethernet: <default(sizeof(ethg) > 0 ? "EthernetEncapsulation" : "")> like IEthernetLayer if typename != "" {
            @display("p=375,825");
        }
        ieee8021q: <default("")> like IIeee8021qLayer if typename != "" {
            @display("p=525,825");
        }
        ieee8021r: <default("")> like IIeee8021rLayer if typename != "" {
            @display("p=749.656,824.17004");
        }
        li: MessageDispatcher {
            @display("p=750,900;b=1000,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=750,975,row,150");
        }
        // TODO move wlan interfaces after eth interfaces, but it changes IP address assignment and breaks examples/inet/configurator/complex.ini
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            @display("p=375,1000,row,150;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            @display("p=300,975,row,150;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            @display("p=1081.582,824.17004,row,150;q=txQueue");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunnelInterface {
            @display("p=1007.06805,1061.26,row,150;q=txQueue");
        }
        virt[numVirtInterfaces]: <default("VirtualInterface")> like IVirtualInterface {
            @display("p=1095.13,998.036,row,150;q=txQueue");
        }
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
    connections allowunconnected:
        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        generic.ifIn <-- nl.out++ if exists(generic);
        generic.ifOut --> nl.in++ if exists(generic);

        cb.out++ --> nl.in++;
        llc.upperLayerOut --> nl.in++ if exists(llc);
        llc.upperLayerIn <-- nl.out++ if exists(llc);
        llc.lowerLayerOut --> cb.in++ if exists(llc);
        llc.lowerLayerIn <-- cb.out++ if exists(llc);

        cb.out++ --> bridging.upperLayerIn if exists(bridging);
        bridging.upperLayerOut --> cb.in++ if exists(bridging);

        bridging.lowerLayerOut --> bl.in++ if exists(bridging);
        bl.out++ --> bridging.lowerLayerIn if exists(bridging);

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
        nl.out++ --> cb.in++;
}

