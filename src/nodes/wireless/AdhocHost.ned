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


import inet.transport.IUDP;
import inet.transport.ITCP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWirelessNic;
import inet.base.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.applications.pingapp.PingApp;
import inet.applications.IUDPApp;
import inet.applications.ITCPApp;


//
// A wireless host containing routing, mobility and battery components.
// Supports only IPv4 protocol, TCP and UDP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// - By default contains a single wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.type-name parameter. see:  inet.linklayer.ieee80211 or other 
//   modules implementing ~IWirelessNic
// - Node mobility can be set using **.mobility.type-name
//   see: inet.mobility and ~IMobility
//
module AdhocHost extends WirelessHost
{
    @display("i=device/cellphone");
    wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
}
