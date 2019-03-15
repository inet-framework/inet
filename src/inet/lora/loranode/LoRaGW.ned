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

package inet.lora.loranode;

import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.linklayer.loopback.LoopbackInterface;
import inet.lora.lorabase.LoRaGWNic;
import inet.lora.loraapp.SimpleLoRaApp;
import inet.lora.lorabase.PacketForwarder;
import inet.linklayer.contract.IWiredInterface;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.IUdp;
import inet.networklayer.contract.IRoutingTable;
import inet.common.MessageDispatcher;

module LoRaGW
{
    parameters:
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        string networkLayerType = default("Ipv4NetworkLayer");
        string routingTableType = default("Ipv4RoutingTable");
        int numUdpApps = default(0);
        bool hasUdp = true;
        string udpType = default(firstAvailableOrEmpty("UDP"));

        @display("bgb=829.08,559.48804;i=device/antennatower");
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=51.888,56.4");
        }
        mobility: StationaryMobility {
            @display("p=51.888,143.256");
        }
        LoRaGWNic: LoRaGWNic {
            @display("p=160.17601,442.17603");
        }
        packetForwarder: PacketForwarder {
            @display("p=404.95203,69.936005;is=vl");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredInterface {
            parameters:
                @display("p=423,433.152,row,60;q=txQueue");
        }
        udp: <udpType> like IUdp if hasUdp {
            parameters:
                @display("p=497.44803,166.944");
        }
        networkLayer: <networkLayerType> like INetworkLayer {
            @display("p=462.48,267.336");
        }
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=51.888,266.208;is=s");
        }


        // Message dispacher

        at: MessageDispatcher {
            parameters:
                @display("p=462,144;b=627,5,,,,1");
        }

        tn: MessageDispatcher {
            parameters:
                @display("p=479.40002,234.62401;b=627,5,,,,1");
        }

        nl: MessageDispatcher {
            parameters:
                @display("p=423,347.424;b=664,5,,,,1");
        }


    connections allowunconnected:
        packetForwarder.lowerLayerOut --> LoRaGWNic.upperLayerIn;
        LoRaGWNic.upperLayerOut --> packetForwarder.lowerLayerIn;
        packetForwarder.udpOut --> at.in++;
        packetForwarder.udpIn <-- at.out++;
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
            eth[i].upperLayerOut --> nl.in++;
            eth[i].upperLayerIn <-- nl.out++;
        }

        networkLayer.ifIn <-- nl.out++;
        networkLayer.ifOut --> nl.in++;

        tn.out++ --> networkLayer.transportIn;
        tn.in++ <-- networkLayer.transportOut;

        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;


        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;


}
