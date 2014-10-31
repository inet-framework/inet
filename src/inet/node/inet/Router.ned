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

package inet.node.inet;

import inet.routing.bgpv4.BGPRouting;
import inet.routing.ospfv2.OSPFRouting;
import inet.routing.rip.RIPRouting;
import inet.transportlayer.ITCP;
import inet.transportlayer.IUDP;


//
// IPv4 router that supports wireless, Ethernet, PPP and external interfaces.
// By default, no wireless and external interfaces are added; the number of
// Ethernet and PPP ports depends on the external connections.
//
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full-duplex connections are supported
//   only (twisted pair). Set **.eth.typename="EthernetInterface" for
//   a full/half-duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessNic
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext.typename in the INI file. see: ~ExtInterface and ~IExternalNic
// - PPP queueing type can be set using **.ppp.queueType see: ~PPPInterface
// - If wireless card is present, node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
// By default, dynamic routing is not supported. Specific routing protocols can
// be added by setting the hasOSPF/hasRIP/hasBGP parameters.
//
module Router extends NodeBase
{
    parameters:
        @display("i=abstract/router");
        forwarding = true;
        bool hasOSPF = default(false);
        bool hasRIP = default(false);
        bool hasBGP = default(false);
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default("UDP");
    submodules:
        ospf: OSPFRouting if hasOSPF {
            parameters:
                @display("p=465,287,row");
        }
        bgp: BGPRouting if hasBGP {
            parameters:
                ospfRoutingModule = default(hasOSPF ? "^.ospf" : "");
                @display("p=214,80;i=block/network");
        }
        tcp: <tcpType> like ITCP if hasBGP {
            parameters:
                @display("p=214,180;i=block/transport");
        }
        rip: RIPRouting if hasRIP {
            parameters:
                @display("p=404,80");
        }
        udp: <udpType> like IUDP if hasRIP {
            @display("p=404,180");
        }
    connections allowunconnected:

        if hasOSPF {
            ospf.ipOut --> networkLayer.transportIn++;
            ospf.ipIn <-- networkLayer.transportOut++;
        }

        if hasBGP {
            bgp.tcpOut --> tcp.appIn++;
            bgp.tcpIn <-- tcp.appOut++;

            tcp.ipOut --> networkLayer.transportIn++;
            tcp.ipIn <-- networkLayer.transportOut++;
        }

        if hasRIP {
            rip.udpOut --> udp.appIn++;
            rip.udpIn <-- udp.appOut++;

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;
        }
}
