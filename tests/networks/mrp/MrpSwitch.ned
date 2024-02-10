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

package inet.tests.mrp;


import inet.applications.contract.IApp;
import inet.clock.contract.IClock;
import inet.common.IMeasurer;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IMacForwardingTable;
import inet.linklayer.contract.IMediaRedundancy;
import inet.linklayer.contract.ISpanningTree;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8021q.IIeee8021qLayer;
import inet.linklayer.ieee8021r.IIeee8021rLayer;
import inet.linklayer.ieee8022.IIeee8022Llc;
import inet.protocolelement.measurement.IMeasurementLayer;
import inet.networklayer.common.InterfaceTable;
import inet.node.contract.IEthernetNetworkNode;
import inet.protocolelement.contract.IProtocolLayer;

module MrpSwitch like IEthernetNetworkNode
{
    parameters:
        bool hasMrp = default(true);
        string MediaRedundancy = default("MediaRedundancyNode");
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        int numApps = default(0);
        bool hasStatus = default(false);
        bool hasStp = default(false);
        bool hasGptp = default(false);
        bool hasCutthroughSwitching = default(false);
        string fcsMode @enum("declared","computed") = default("declared");
        string spanningTreeProtocol = default("Stp");
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        eth[*].encap.typename = default("");
        *.fcsMode = this.fcsMode;
        **.interfaceTableModule = default(absPath(".interfaceTable"));
        **.macTableModule = default(absPath(".macTable"));
        **.mrpRelayModule = default(absPath(".bridging"));
        *.clockModule = default(exists(clock) ? absPath(".clock") : "");

        llc.registerProtocol = true;
        eth[*].mac.promiscuous = default(true);
        ethernet.*.promiscuous = default(true);
        ethernet.registerProtocol = default(true);
    gates:
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn);
    submodules:
        mrp: <MediaRedundancy> like IMediaRedundancy if hasMrp {
            @display("p=516,49");
        }
        mrpMeasurementLayer: <default("")> like IMeasurementLayer {
            @display("p=604,100");
        }
        macTable: <default("MrpMacForwardingTable")> like IMacForwardingTable {
            @display("p=100,100;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=100,200;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if hasMrp {
            @display("p=100,300;is=s");
        }
        //        l2NodeConfiguratorMrp: L2NodeConfigurator if hasMrp {
        //            @display("p=100,300;is=s");
        //        }
        status: NodeStatus if hasStatus {
            @display("p=100,400;is=s");
        }
        clock: <default(hasGptp ? "SettableClock" : "")> like IClock if typename != "" {
            @display("p=100,500;is=s");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=100,600;is=s");
        }
        measurer: <default("")> like IMeasurer if typename != "" {
            @display("p=125,700;is=s");
        }
        stp: <spanningTreeProtocol> like ISpanningTree if hasStp {
            @display("p=678,49");
        }
        gptp: <default("Gptp")> like IApp if hasGptp {
            @display("p=817,49");
            gptpNodeType = default("BRIDGE_NODE"); // @enum("gptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        }
        app[numApps]: <> like IApp {
            parameters:
                @display("p=952,49,row,150");
        }
        sc: MessageDispatcher {
            @display("p=800,150;b=1200,5");
        }
        llc: <default("Ieee8022Llc")> like IIeee8022Llc if typename != "" {
            @display("p=700,225");
        }
        cb: MessageDispatcher {
            @display("p=800,300;b=1200,5");
        }
        bridging: <default("MrpRelay")> like IProtocolLayer {
            @display("p=800,375;is=m");
        }
        bl: MessageDispatcher {
            @display("p=800,450;b=1200,5");
        }
        ethernet: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" {
            @display("p=500,525");
        }
        ieee8021q: <default("Ieee8021qProtocol")> like IIeee8021qLayer if typename != "" {
            @display("p=700,525");
        }
        ieee8021r: <default("Ieee8021rProtocol")> like IIeee8021rLayer if typename != "" {
            @display("p=900,525");
        }
        li: MessageDispatcher {
            @display("p=800,600;b=1200,5");
        }
        eth[sizeof(ethg)]: <default(hasCutthroughSwitching ? "EthernetCutthroughInterface" : "EthernetInterface")> like IEthernetInterface {
            @display("p=250,750,row,150;q=txQueue");
        }
    connections:
        if hasMrp {
                mrp.relayIn <-- mrpMeasurementLayer.upperLayerOut;
                mrp.relayOut --> mrpMeasurementLayer.upperLayerIn;
                mrpMeasurementLayer.lowerLayerOut --> sc.in++;
                mrpMeasurementLayer.lowerLayerIn <-- sc.out++;
        }
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        ieee8021q.lowerLayerOut --> li.in++ if exists(ieee8021q);
        li.out++ --> ieee8021q.lowerLayerIn if exists(ieee8021q);

        ieee8021r.lowerLayerOut --> li.in++ if exists(ieee8021r);
        li.out++ --> ieee8021r.lowerLayerIn if exists(ieee8021r);

        bl.out++ --> li.in++;
        li.out++ --> bl.in++;

        llc.upperLayerOut --> sc.in++ if exists(llc);
        sc.out++ --> llc.upperLayerIn if exists(llc);

        sc.out++ --> cb.in++;
        cb.out++ --> sc.in++;

        cb.out++ --> llc.lowerLayerIn if exists(llc);
        llc.lowerLayerOut --> cb.in++ if exists(llc);

        cb.out++ --> bridging.upperLayerIn;
        bridging.upperLayerOut --> cb.in++;

        ieee8021q.upperLayerOut --> bl.in++ if exists(ieee8021q);
        bl.out++ --> ieee8021q.upperLayerIn if exists(ieee8021q);

        ieee8021r.upperLayerOut --> bl.in++ if exists(ieee8021r);
        bl.out++ --> ieee8021r.upperLayerIn if exists(ieee8021r);

        bl.out++ --> ethernet.upperLayerIn if exists(ethernet);
        ethernet.upperLayerOut --> bl.in++ if exists(ethernet);

        bridging.lowerLayerOut --> bl.in++;
        bl.out++ --> bridging.lowerLayerIn;

        for i=0..sizeof(ethg)-1 {
            li.out++ --> eth[i].upperLayerIn;
            li.in++ <-- eth[i].upperLayerOut;
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }

        if hasStp {
            stp.relayIn <-- sc.out++;
            stp.relayOut --> sc.in++;
        }

        if hasGptp {
            gptp.socketOut --> sc.in++;
            sc.out++ --> gptp.socketIn;
        }

        for i=0..numApps-1 {
            app[i].socketOut --> sc.in++;
            app[i].socketIn <-- sc.out++;
        }


}
