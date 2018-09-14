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

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IMacAddressTable;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.contract.IWirelessInterface;
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
// The wlan[*].mgmt.typename can be configured for different management types
// currently it can be: ~Ieee80211MgmtApSimplified and ~Ieee80211MgmtAp.
// By default it is ~Ieee80211MgmtAp. The Simplified version does not
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
        int numWlanInterfaces = default(1);               // the number of radios in the access point
        bool hasStatus = default(false);
        wlan[*].mgmt.typename = default("Ieee80211MgmtAp");
        wlan[*].llc.typename = default("Ieee80211Portal");
        wlan[*].agent.typename = default("");
        wlan[*].radio.antenna.mobilityModule = default("^.^.^.mobility");
        eth[*].encap.typename = "EtherEncapDummy";
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        relayUnit.hasStp = false;
    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            parameters:
                @display("p=100,100;is=s");
        }

        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,200;is=s");
        }
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
                @display("p=100,400;is=s");
        }
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            parameters:
                @display("p=100,300;is=s");
        }
        relayUnit: <default(firstAvailable("Ieee8021dRelay","MacRelayUnit"))> like IMacRelayUnit if sizeof(ethg)+numWlanInterfaces > 1 && typename != "" {
            parameters:
                @display("p=400,100");
        }
        nl: MessageDispatcher if sizeof(ethg)+numWlanInterfaces > 1 && exists(relayUnit) {
            parameters:
                @display("p=800,200;b=1200,5,,,,1");
        }
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            parameters:
                @display("p=250,300,row,150;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                mac.promiscuous = true;
                @display("p=850,300,row;q=txQueue");
        }
    connections allowunconnected:
        nl.out++--> relayUnit.ifIn if sizeof(ethg)+numWlanInterfaces > 1 && exists(relayUnit);
        nl.in++ <-- relayUnit.ifOut if sizeof(ethg)+numWlanInterfaces > 1 && exists(relayUnit);
        // connections to network outside
        // wireless interfaces MUST be connected first (i.e. ports 0..numWlanInterfaces-1)
        // because broadcasts must be handled differently for wireless IFs by the relayUnit
        for i=0..numWlanInterfaces-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> nl.in++ if sizeof(ethg)+numWlanInterfaces > 1 && exists(relayUnit);
            wlan[i].upperLayerIn <-- nl.out++ if sizeof(ethg)+numWlanInterfaces > 1 && exists(relayUnit);
        }
        // ethernet must be connected only AFTER wireless ports
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
            eth[i].upperLayerIn <-- nl.out++ if sizeof(ethg)+numWlanInterfaces>1;
            eth[i].upperLayerOut --> nl.in++ if sizeof(ethg)+numWlanInterfaces>1;
        }
}

