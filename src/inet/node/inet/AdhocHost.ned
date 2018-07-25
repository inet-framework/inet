//
// Copyright (C) OpenSim Ltd.
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

package inet.node.inet;

//
// A wireless host containing routing, mobility and energy components. Supports
// IPv4 network protocol, TCP, UDP, and SCTP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// - By default contains a single wireless card, however it can be configured
//   by the numWlanInterfaces parameter. Wirless card type is configured by the
//   **.wlan[*].typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessInterface
// - Node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
module AdhocHost extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        wlan[*].mgmt.typename = default("Ieee80211MgmtAdhoc");
        wlan[*].agent.typename = default("");
        forwarding = default(true);
}

