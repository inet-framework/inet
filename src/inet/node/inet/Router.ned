//
// Copyright (C) OpenSim Ltd.
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

import inet.applications.contract.IApp;
import inet.node.base.ApplicationLayerNodeBase;
import inet.routing.contract.IBgp;
import inet.routing.contract.IOspf;
import inet.routing.contract.IPim;

//
// Ipv4 router that supports wireless, Ethernet, PPP and external interfaces.
// By default, no wireless and external interfaces are added; the number of
// Ethernet and PPP ports depends on the external connections.
//
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full-duplex connections are supported
//   only (twisted pair). Set **.eth.typename="EthernetInterface" for
//   a full/half-duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numWlanInterfaces parameter. Wirless card type is configured by the
//   **.wlan.typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessInterface
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext.typename in the INI file. see: ~ExtInterface and ~IExternalInterface
// - PPP queueing type can be set using **.ppp.queueType see: ~PppInterface
// - If wireless card is present, node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
// By default, dynamic routing is not supported. Specific routing protocols can
// be added by setting the hasOSPF/hasRIP/hasBGP parameters.
//
module Router extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=abstract/router");
        @figure[submodules];
        forwarding = true;
        bool hasOSPF = default(false);
        bool hasRIP = default(false);
        bool hasBGP = default(false);
        bool hasPIM = default(false);
        bool hasDHCP = default(false);
        hasUdp = default(hasRIP || hasDHCP);
        hasTcp = default(hasBGP);
        *.routingTableModule = default("^.ipv4.routingTable");

    submodules:
        ospf: <"Ospf"> like IOspf if hasOSPF {
            parameters:
                @display("p=800,300,row");
        }
        bgp: <"Bgp"> like IBgp if hasBGP {
            parameters:
                ospfRoutingModule = default(hasOSPF ? "^.ospf" : "");
                @display("p=600,100");
        }
        rip: <"Rip"> like IApp if hasRIP {
            parameters:
                @display("p=700,100");
        }
        pim: <"Pim"> like IPim if hasPIM {
            parameters:
                @display("p=700,300");
        }
        dhcp: <"DhcpServer"> like IApp if hasDHCP {
            parameters:
                @display("p=800,100");
        }

    connections allowunconnected:
        ospf.ipOut --> tn.in++ if hasOSPF;
        ospf.ipIn <-- tn.out++ if hasOSPF;

        bgp.socketOut --> at.in++ if hasBGP;
        bgp.socketIn <-- at.out++ if hasBGP;

        rip.socketOut --> at.in++ if hasRIP;
        rip.socketIn <-- at.out++ if hasRIP;

        pim.networkLayerOut --> tn.in++ if hasPIM;
        pim.networkLayerIn <-- tn.out++ if hasPIM;

        dhcp.socketOut --> at.in++ if hasDHCP;
        dhcp.socketIn <-- at.out++ if hasDHCP;
}
