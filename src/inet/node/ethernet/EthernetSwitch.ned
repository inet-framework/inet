//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.node.ethernet;

import inet.clock.contract.IClock;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IMacAddressTable;
import inet.linklayer.contract.ISpanningTree;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8021q.IIeee8021qLayer;
import inet.linklayer.ieee8021r.IIeee8021rLayer;
import inet.networklayer.common.InterfaceTable;
import inet.protocolelement.contract.IProtocolLayer;

//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
// By default used half duples CSMA/CD mac
//
module EthernetSwitch
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        int numPcapRecorders = default(0);
        bool hasStatus = default(false);
        bool hasStp = default(false);
        bool enableCutthrough = default(false);
        string fcsMode @enum("declared","computed") = default("declared");
        string spanningTreeProtocol = default("Stp");
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        eth[*].encap.typename = default("OmittedEthernetEncapsulation");
        *.fcsMode = fcsMode;
        **.interfaceTableModule = default(absPath(".interfaceTable"));
        **.macTableModule = default(absPath(".macTable"));

        bridging.hasStp = hasStp;
        eth[*].mac.promiscuous = default(true);
        ethernet.*.promiscuous = default(true);

    gates:
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,400;is=s");
        }
        clock: <default("")> like IClock if typename != "" {
            parameters:
                @display("p=100,600;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if hasStp {
            @display("p=100,300;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=100,200;is=s");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=100,500;is=s");
        }
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            @display("p=100,100;is=s");
        }
        bridging: <default(firstAvailable("Ieee8021dRelay","MacRelayUnit"))> like IProtocolLayer {
            @display("p=800,275;is=m");
        }
        stp: <spanningTreeProtocol> like ISpanningTree if hasStp {
            @display("p=500,100");
        }
        nr: MessageDispatcher {
            parameters:
                @display("p=800,200;b=1200,5");
        }
        li: MessageDispatcher {
            parameters:
                @display("p=800,500;b=1200,5");
        }
        eth[sizeof(ethg)]: <default(enableCutthrough ? "EthernetCutthroughInterface" : "EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=250,700,row,150;q=txQueue");
        }
        rl: MessageDispatcher {
            parameters:
                @display("p=800,350;b=1200,5");
        }
        ethernet: <default("")> like IEthernetLayer if typename != "" {
            @display("p=500,425");
        }
        ieee8021q: <default("Ieee8021qProtocol")> like IIeee8021qLayer if typename != "" {
            @display("p=700,425");
        }
        ieee8021r: <default("Ieee8021rProtocol")> like IIeee8021rLayer if typename != "" {
            @display("p=900,425");
        }
        cutthrough: MessageDispatcher if enableCutthrough {
            @display("p=800,600;b=1200,5");
        }
    connections:
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        ieee8021q.lowerLayerOut --> li.in++ if exists(ieee8021q);
        li.out++ --> ieee8021q.lowerLayerIn if exists(ieee8021q);

        ieee8021r.lowerLayerOut --> li.in++ if exists(ieee8021r);
        li.out++ --> ieee8021r.lowerLayerIn if exists(ieee8021r);

        for i=0..sizeof(ethg)-1 {
            eth[i].cutthroughOut --> cutthrough.in++ if enableCutthrough;
            eth[i].cutthroughIn <-- cutthrough.out++ if enableCutthrough;
        }

        rl.out++ --> li.in++;
        li.out++ --> rl.in++;

        ieee8021q.upperLayerOut --> rl.in++ if exists(ieee8021q);
        rl.out++ --> ieee8021q.upperLayerIn if exists(ieee8021q);

        ieee8021r.upperLayerOut --> rl.in++ if exists(ieee8021r);
        rl.out++ --> ieee8021r.upperLayerIn if exists(ieee8021r);

        rl.out++ --> ethernet.upperLayerIn if exists(ethernet);
        ethernet.upperLayerOut --> rl.in++ if exists(ethernet);

        bridging.lowerLayerOut --> rl.in++;
        rl.out++ --> bridging.lowerLayerIn;

        for i=0..sizeof(ethg)-1 {
            li.out++ --> eth[i].upperLayerIn;
            li.in++ <-- eth[i].upperLayerOut;
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }

        if hasStp {
            stp.relayIn <-- nr.out++; // relayLayer.stpOut;
            stp.relayOut --> nr.in++; // relayLayer.stpIn;
        }
        bridging.upperLayerOut --> nr.in++;
        nr.out++ --> bridging.upperLayerIn;
}

