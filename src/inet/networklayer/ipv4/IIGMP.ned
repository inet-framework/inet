//
// Copyright (C) 2012 Andras Varga
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
package inet.networklayer.ipv4;

//
// Module interface for IGMP modules.
//
// An IGMP module sends notifications about multicast group memberships in hosts,
// and processes these notifications in multicast routers.
//
// Currently there are 3 versions specified in RFC 1112 (IGMPv1), RFC 2236 (IGMPv2),
// and RFC 3376 (IGMPv3).
//
// IGMP packets are transported by the IP layer connected to the ipIn/ipOut
// gates. Some multicast routing protocols (e.g. DVMRP) also communitacate with
// IGMP messages. They must be connected to the routerIn/routerOut gates,
// the IGMP module will pass up the unrecognized IGMP packets.
//
// @see IGMPv2
//
moduleinterface IIGMP
{
    @display("i=block/cogwheel_s");

    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);

        input routerIn @labels(IPv4ControlInfo/up);
        output routerOut @labels(IPv4ControlInfo/down);
}
