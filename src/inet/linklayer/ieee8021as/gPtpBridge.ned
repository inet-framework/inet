//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package ieee8021as;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IMACAddressTable;
import inet.linklayer.contract.IMACRelayUnit;
import inet.linklayer.contract.ISpanningTree;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.linklayer.ethernet.switch.MACRelayUnit;
import inet.networklayer.common.InterfaceTable;

module GPtpBridge
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string spanningTreeProtocol = default("");
        string relayUnitType = default(firstAvailable("Ieee8021dRelay","MACRelayUnit")); // type of the ~IMACRelayUnit;
        string macTableType = default("MACAddressTable"); // type of the ~IMACAddressTable
        string nodeType = default("BridgeNode");
        eth[*].encapType = "EtherEncapDummy";
        eth[*].csmacdSupport = csmacdSupport;
        eth[*].macType = macType;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        
        // 11 - Grandmaster, 12 - Bridge, 13 - Slave
        int gPtpNodeType = 12;	
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,127;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if spanningTreeProtocol != "" {
            @display("p=50,181");
        }
        interfaceTable: InterfaceTable {
            @display("p=50,65;is=s");
        }
        macTable: <macTableType> like IMACAddressTable {
            @display("p=50,19;is=s");
        }
        relayUnit: <relayUnitType> like IMACRelayUnit {
            parameters:
                @display("p=263,50;is=m");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
        }
        stp: <spanningTreeProtocol> like ISpanningTree if spanningTreeProtocol != "" {
            @display("p=168,50");
        }
        eth[sizeof(ethg)]: EthernetInterfaceGPTP {
            parameters:
                mac.promiscuous = true;
                @display("p=263,150,row;q=txQueue");
        }
        tableGPTP: TableGPTP {
            @display("p=149,127");
        }
        clock: Clock {
            @display("p=149,188");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerIn <-- relayUnit.ifOut++;
            eth[i].upperLayerOut --> relayUnit.ifIn++;
            eth[i].phys <--> ethg[i];
            eth[i].gptpLayerOut --> tableGPTP.gptpLayerIn++;
            eth[i].gptpLayerIn <-- tableGPTP.gptpLayerOut++;
        }

        if spanningTreeProtocol != "" {
            stp.relayIn <-- relayUnit.stpOut;
            stp.relayOut --> relayUnit.stpIn;
        }
}
