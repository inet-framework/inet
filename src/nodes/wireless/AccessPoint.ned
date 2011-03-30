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

import inet.linklayer.IWirelessNic;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.IEtherMAC;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;


//
// A generic access point supporting multiple wireless radios, and
// multiple ethernet ports. The type of the ethernet MAC, relay Unit
// and wireless card can be specified as parameters.
// By default the access point is stationary (NullMobility), but can be
// changed also if needed.
//
module AccessPoint
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/accesspoint");
        string mobilityType = default("NullMobility"); // how the node moves around. see: inet.mobility
        string relayUnitType = default(haveClass("MACRelayUnitNP")?"MACRelayUnitNP":"MACRelayUnit_None"); // type of the IMACRelayUnit (MACRelayUnitNP, MACRelayUnitPP, etc)
        string wlanNicType = default("Ieee80211NicAP"); // used wireless interfaces. see:  inet.linklayer.ieee80211 or other modules implementing IWirelessNic. Use only the NICs for access points
        string ethMacType = default("EtherMACFullDuplex"); // used ethernet interfaces. see: IEtherMAC and IEtherMACFullDuplex for simplified, full dupley only mac
        int numRadios = default(1);               // the number of radios in the access point
    gates:
        input radioIn[numRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        //@ TODO make the relayUnit optional too (present only if ethSize+wlanSize>1)
        relayUnit: <relayUnitType> like IMACRelayUnit {
            parameters:
                @display("p=151,121");
        }
        wlan[numRadios]: <wlanNicType> like IWirelessNic {
            parameters:
                @display("p=89,225;q=queue");
        }
        eth[sizeof(ethg)]: <ethMacType> like IEtherMAC {
            parameters:
                promiscuous = true;
                txQueueLimit = 1000; // increase if needed
                queueModule = "";
                @display("p=208,225,row;q=queue");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=59,43");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=59,107");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].uppergateOut --> relayUnit.lowerLayerIn++;
            wlan[i].uppergateIn <-- relayUnit.lowerLayerOut++;
        }
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> ethg[i];
            eth[i].upperLayerIn <-- relayUnit.lowerLayerOut++;
            eth[i].upperLayerOut --> relayUnit.lowerLayerIn++;
        }
}
