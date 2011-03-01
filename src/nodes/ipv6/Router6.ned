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

package inet.nodes.ipv6;

import inet.util.NAMTraceWriter;
import inet.networklayer.ipv6.RoutingTable6;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.ext.ExtInterface;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;

//
// IPv6 router.
//
module Router6
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=abstract/router");
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        string mobilityType = default("NoMobility"); // how the node moves around. MUST be specified if the host contains a wireless interface. see: inet.mobility
        string wlanNicType = default("Ieee80211NicSTA"); // used wireless interfaces. see:  inet.linklayer.ieee80211 or other modules implementing IWirelessNic
        string ethNicType = default("EthernetInterfaceFullDuplex"); // used ethernet interfaces. see: EtherInterface (full/half duplex) or EtherInterface2 for simplified, full duplex only
        string pppNicType = default("PPPInterface"); // used pp interfaces. see: PPPInerface and PPPInterfaceNoQueue
        int namid = default(-1);
        xml routingTableFile;  //# TODO provide a default
    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=375,54");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=57,54");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=177,54");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = true;
                routingTableFile = routingTableFile;
                @display("p=281,54");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=468,54");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=242,143;q=queue");
        }
        wlan[numRadios]: <wlanNicType> like IWirelessNic {
            parameters:
                @display("p=72,242;q=queue");
        }
        eth[sizeof(ethg)]: <ethNicType> like IWiredNic {
            parameters:
                @display("p=195,242,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <pppNicType> like IWiredNic {
            parameters:
                @display("p=320,242,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: ExtInterface {
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

