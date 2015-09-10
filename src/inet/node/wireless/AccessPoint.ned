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

package inet.node.wireless;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IMACAddressTable;
import inet.linklayer.contract.IMACRelayUnit;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.mobility.contract.IMobility;
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
        @networkNode();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/accesspoint");
        string mobilityType = default("StationaryMobility"); // how the node moves around. see: inet.mobility
        string relayUnitType = default(firstAvailable("Ieee8021dRelay","MACRelayUnit")); // type of the IMACRelayUnit (MACRelayUnit, Ieee8021dRelay, etc)
        string macTableType = default("MACAddressTable"); // type of the ~IMACAddressTable
        int numRadios = default(1);               // the number of radios in the access point
        bool hasStatus = default(false);
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
        eth[*].encapType = "EtherEncapDummy";
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    gates:
        input radioIn[numRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=51,43");
        }

        interfaceTable: InterfaceTable {
            @display("p=51,121;is=s");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=208,43");
        }
        macTable: <macTableType> like IMACAddressTable {
            @display("p=73,19;is=s");
        }
        relayUnit: <relayUnitType> like IMACRelayUnit if sizeof(ethg)+numRadios>1 && relayUnitType != "" {
            parameters:
                @display("p=151,121");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=89,225,row;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=208,225,row;q=txQueue");
        }
    connections allowunconnected:
        // connections to network outside
        // wireless interfaces MUST be connected first (i.e. ports 0..numRadios-1)
        // because broadcasts must be handled differently for wireless IFs by the relayUnit
        for i=0..numRadios-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> relayUnit.ifIn++ if sizeof(ethg)+numRadios>1;
            wlan[i].upperLayerIn <-- relayUnit.ifOut++ if sizeof(ethg)+numRadios>1;
        }
        // ethernet must be connected only AFTER wireless ports
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> ethg[i];
            eth[i].upperLayerIn <-- relayUnit.ifOut++ if sizeof(ethg)+numRadios>1;
            eth[i].upperLayerOut --> relayUnit.ifIn++ if sizeof(ethg)+numRadios>1;
        }
}
