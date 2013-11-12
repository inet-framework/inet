//
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

package inet.nodes.inet;

import inet.base.NotificationBoard;
import inet.battery.IBattery;
import inet.linklayer.IExternalNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.IMobility;
import inet.networklayer.common.IRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.status.NodeStatus;
import inet.util.PcapRecorder;


//
// Contains the common lower layers (linklayer and networklayer)
// of Router, StandardHost, WirelessHost etc.
//
// NOTE:
//    - IPForward is turned on by default.
//    - use this model only as a base type. In simulations use ~Router,
//      ~StandardHost, ~WirelessHost and ~AdhocHost
//
module NodeBase
{
    parameters:
        @display("bgb=,448");
        @node;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        bool IPForward = default(true);               // TODO should we remove these parameters from the routingTable
        bool forwardMulticast = default(false);
        string batteryType = default("");
        routingTable.IPForward = IPForward;
        routingTable.forwardMulticast = forwardMulticast; //      and use the parameters of the node in the C++ code?
    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,50");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,155;is=s");
        }

        battery: <batteryType> like IBattery if batteryType != "" {
            parameters:
                @display("p=53,77;i=block/plug;is=s");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=53,111");
        }

        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=329,287;q=queue");
        }

		// routing table
        routingTable: <routingTableType> like IRoutingTable {
            parameters:
                @display("p=53,225;is=s");
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,287;is=s");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=53,338,r,10");
        }

        lo0: LoopbackInterface {
            @display("p=78,406");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=216,406,row,60;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=368,406,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=558,406,row,60;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=149,406,row,10;q=txQueue;i=block/ifcard");
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
}

