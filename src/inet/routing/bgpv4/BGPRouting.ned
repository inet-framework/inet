//
// Copyright (C) 2010 Helene Lageber
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

package inet.routing.bgpv4;

import inet.applications.ITCPApp;


//
// Implements BGP Version 4.
// TODO config file!
//
// The model implements RFC 4271, with the following limitations:
//   - NOTIFICATION message is not implemented
//   - MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer
//     are not implemented
//   - Optional UPDATE message Path Attributes are not implemented
//   - Optional Final State Machine events are not implemented
//
// Implemented features by RFC 4271 chapters:
// - 3. Summary of Operation (Routes & routing Information Base) -- implemented
// - 4. Message Formats:
//     - Header, OPEN, UPDATE & KEEPALIVE -- implemented
//     - NOTIFICATION -- not implemented
// - 5. Path Attributes:
//     - ORIGIN, AS_PATH, NEXT_HOP -- implemented
//     - MULTI_EXIT_DISC, LOCAL_PREF, ATOMIC_AGGRGATE, AGGREGATOR -- not implemented
// - 6. BGP Error Handling -- not implemented
// - 7. BGP Version Negotiation -- not implemented
// - 8. Event for the BGP FSM -- implemented except optional ones
// - 9. UPDATE Message Handling:
//     - Decision Process -- implemented
//     - Update-Send Process -- implemented except Controlling Routing Traffic Overhead
// - 10. BGP timers:
//     - ConnectRetryTimer, Holdtimer, KeepAliveTimer -- implemented
//     - MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer -- not implemented
//
// @author Helene Lageber
//
simple BGPRouting like ITCPApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string ospfRoutingModule;
        @display("i=block/network2");
        xml bgpConfig;
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
    gates:
        input tcpIn;
        output tcpOut;
}


