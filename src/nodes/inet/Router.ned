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

import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IExtInterface;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;

//
// \IP router supporting wireless, ethernet, ppp and external interfaces.
// by default no wireless and external if is configured. The number of
// ethernet and ppp ports depend on the external connections.
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full duplex connections are supported
//   only (twisted pair). Set **.eth.type-name="EthernetInterface" for
//   a full/half duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.type-name parameter. see:  inet.linklayer.ieee80211 or other 
//   modules implementing ~IWirelessNic
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext.type-name in the INI file. see: ~ExtInerface and ~IExtInterface
// - PPP interface type can be set using **.ppp.type-name see: ~PPPInterface 
//   and ~PPPInterfaceNoQueue
// - If wireless card is present, node mobility can be set using **.mobility.type-name
//   see: inet.mobility and ~IMobility
//
module Router
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=abstract/router");
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        string routingFile = default("");
    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=57,54");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=177,54");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=281,54");
        }
        mobility: <default("NoMobility")> like IMobility {
            parameters:
                @display("p=379,54");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=242,143;q=queue");
        }
        wlan[numRadios]: <default("Ieee80211NicSTA")> like IWirelessNic {
            parameters:
                @display("p=72,242;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterfaceFullDuplex")> like IWiredNic {
            parameters:
                @display("p=195,242,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=320,242,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExtInterface {
            parameters:
                @display("p=460,242,row,90;q=txQueue;i=block/ifcard");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].uppergateOut --> networkLayer.ifIn++;
            wlan[i].uppergateIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn++;
            eth[i].netwIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn++;
            ppp[i].netwIn <-- networkLayer.ifOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].netwOut --> networkLayer.ifIn++;
            ext[i].netwIn <-- networkLayer.ifOut++;
        }
}

