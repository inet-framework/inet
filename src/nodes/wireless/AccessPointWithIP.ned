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

package inet.nodes.wireless;

import inet.base.NotificationBoard;
import inet.linklayer.IEtherMAC;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.IWirelessNic;
import inet.linklayer.ethernet.EtherEncap;
import inet.linklayer.ethernet.EthernetInterface;
import inet.mobility.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.nodes.inet.NetworkLayer;


//
// Access Point integrated with an Ethernet switch, and containing
// higher layers for supporting protocols like DHCP.
//
// TODO finish the module! add UDP, TCP, etc.
//
module AccessPointWithIPStack
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/accesspoint;bgb=367,414");
        string mobilityType = default("StationaryMobility"); // how the node moves around. see: inet.mobility
        string relayUnitType = default(haveClass("MACRelayUnitNP")?"MACRelayUnitNP":"MACRelayUnit_None"); // type of the IMACRelayUnit (MACRelayUnitNP, MACRelayUnitPP, etc)
        int numRadios = default(1);               // the number of radios in the access point
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
    gates:
        input radioIn[numRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        relayUnit: <relayUnitType> like IMACRelayUnit if sizeof(ethg)+numRadios>1 {
            parameters:
                @display("p=132,256");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=70,360;q=queue");
        }
        eth[sizeof(ethg)]: <default("EtherMACFullDuplex")> like IEtherMAC {
            parameters:
                promiscuous = true;
                txQueueLimit = 1000; // increase if needed
                queueModule = "";
                @display("p=211,360,row;q=txQueue");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=47,217");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=39,287");
        }
        etherEncap: EtherEncap {
            @display("p=132,178");
        }
        networkLayer: NetworkLayer {
            @display("p=132,104");
        }
    connections allowunconnected:
        // connections to network outside
        // wireless interfaces MUST be connected first (i.e. ports 0..numRadios-1)
        // because broadcasts must be handled differently for wireless IFs by the relayUnit
        for i=0..numRadios-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> relayUnit.lowerLayerIn++ if sizeof(ethg)+numRadios>1;
            wlan[i].upperLayerIn <-- relayUnit.lowerLayerOut++ if sizeof(ethg)+numRadios>1;
        }
        // ethernet must be connected only AFTER wireless ports
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> ethg[i];
            eth[i].upperLayerIn <-- relayUnit.lowerLayerOut++ if sizeof(ethg)+numRadios>1;
            eth[i].upperLayerOut --> relayUnit.lowerLayerIn++ if sizeof(ethg)+numRadios>1;
        }
        relayUnit.lowerLayerOut++ --> etherEncap.lowerLayerIn;
        etherEncap.lowerLayerOut --> relayUnit.lowerLayerIn++;
        etherEncap.upperLayerOut --> networkLayer.ifIn++;
        networkLayer.ifOut++ --> etherEncap.upperLayerIn;
}
