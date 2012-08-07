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

import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.IEtherMAC;
import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;


//
// A generic access point supporting multiple wireless radios, and
// multiple ethernet ports. The type of the ethernet MAC, relay unit
// and wireless card can be specified as parameters.
//
// By default, the access point is stationary (~StationaryMobility),
// but that can also be configured by a parameter.
//
// The wlan[*].mgmtType can be configured for different management types
// currently it can be: ~Ieee80211MgmtAPSimplified and ~Ieee80211MgmtAP.
// By default it is ~Ieee80211MgmtAP. The Simplified version does not
// support channel scanning, authentication and association .
// In this case, nodes must explicitly specify the hardware address
// of the wlan card they want to be associated with.
//
module AccessPoint
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/accesspoint");
        string mobilityType = default("StationaryMobility"); // how the node moves around. see: inet.mobility
        string relayUnitType = default(haveClass("MACRelayUnitNP")?"MACRelayUnitNP":"MACRelayUnit_None"); // type of the IMACRelayUnit (MACRelayUnitNP, MACRelayUnitPP, etc)
        int numRadios = default(1);               // the number of radios in the access point
        bool withInterfaceTable = default(false);    // module has interfaceTable if enabled
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
    gates:
        input radioIn[numRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        interfaceTable: InterfaceTable if withInterfaceTable {
            @display("p=59,121");
        }
        relayUnit: <relayUnitType> like IMACRelayUnit if sizeof(ethg)+numRadios>1 {
            parameters:
                @display("p=151,121");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=89,225;q=queue");
        }
        eth[sizeof(ethg)]: <default("EtherMACFullDuplex")> like IEtherMAC {
            parameters:
                promiscuous = true;
                txQueueLimit = 1000; // increase if needed
                queueModule = "";
                @display("p=208,225,row;q=txQueue");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=59,43");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=169,43");
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
}
