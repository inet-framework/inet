//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package ieee8021as;

import inet.common.lifecycle.NodeStatus;
import inet.common.packet.PcapRecorder;
import inet.linklayer.contract.IExternalNic;
import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.IRoutingTable;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyManagement;
import inet.power.contract.IEnergyStorage;
import inet.node.inet.INetworkNode;

module GPtpEndstation like INetworkNode
{
    parameters:
        @display("bgb=,448");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        int numTunInterfaces = default(0);
        string osgModel = default(""); // 3D model for OSG visualization, no 3D model by default
        string osgModelColor = default(""); // tint color, no colorization by default
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        bool forwarding = default(true);
        bool multicastForwarding = default(false);
        string energyStorageType = default("");
        string energyManagementType = default("");
        string energyGeneratorType = default("");
        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");

    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=53,16");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=53,60;i=block/plug;is=s");
        }
        energyManagement: <energyManagementType> like IEnergyManagement if energyManagementType != "" {
            parameters:
                @display("p=53,106;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=53,153;i=block/plug;is=s");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=53,200");
        }
        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=329,287;q=queue");
        }
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=53,250;is=s");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,300;is=s");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=53,350,r,10");
        }
        lo0: LoopbackInterface {
            @display("p=78,406");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=243,406,row,60;q=queue");
        }
        eth[sizeof(ethg)]: EthernetInterfaceGPTP {  
            parameters:
                @display("p=420,406,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=558,406,row,60;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=144,445,row,10;q=txQueue;i=block/ifcard");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=329,445,row,10;q=txQueue;i=block/ifcard");
        }

        clock: Clock {
            @display("p=558,331");
        }
        tableGPTP: TableGPTP {
            @display("p=558,259");
        }
    connections allowunconnected:
        // connections to network outside
        networkLayer.ifOut++ --> lo0.upperLayerIn;
        lo0.upperLayerOut --> networkLayer.ifIn++;

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
            eth[i].gptpLayerOut --> tableGPTP.gptpLayerIn++;
            eth[i].gptpLayerIn <-- tableGPTP.gptpLayerOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> networkLayer.ifIn++;
            ext[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> networkLayer.ifIn++;
            tun[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}
