//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.modes;

//
// Implementation of PIM-SM protocol (RFC 4601).
//
// PIM-SM is a multicast routing protocol which
// build a shared multicast forwarding three rooted at a Rendezvous Point (RP)
// per group, and a source specific forwarding trees from the sources to the RP.
// It is also capable to switch to a shortest path tree, so that traffic from
// sources to receivers do not have to pass through the RP.
//
// The module must access the ~PIMInterfaceTable and ~PIMNeighbortable
// modules, their path is given as the 'pimInterfaceTableModule' and
// 'pimNeighborTableModule' parameters.
//
// The 'RP' parameter specifies the address of the Rendezvous Point.
// Currently only one global RP is supported.
//
// If there are multiple PIM routers on a multi-access LAN, only one
// of them will be responsible for forwarding datagrams to the LAN.
// This will be the router with the highest IP address except if
// all routers has 'designatedRouterPriority' set, in which case
// the highest priority wins (or highest IP address on ties).
//
// Other parameters set the time constants to the values specified
// in RFC 4601. They should not be changed except for testing.
//
// Limitations:
// - Only one global RP is supported and it must be specified statically.
//   PIM Bootstrap and RP discovery is not yet implemented.
// - Switchover to the shortest path tree is not supported.
// - Source specific excludes in the shared tree are not supported.
//
simple PIMSM
{
    parameters:
        @display("i=block/network2");
        string interfaceTableModule;
        string routingTableModule;
        string pimInterfaceTableModule;
        string pimNeighborTableModule;
        string RP = default("");
        int designatedRouterPriority = default(-1);

        volatile double triggeredHelloDelay @unit(s) = uniform(0s,5s);
        double helloPeriod @unit(s) = 30s;
        double holdTime @unit(s) = default(3.5 * helloPeriod);
        double joinPrunePeriod @unit(s) = default(60s); // period between Join/Prune messages (called t_periodic in the RFC)
        double defaultOverrideInterval @unit(s) = 2.5s;
        double defaultPropagationDelay @unit(s) = 0.5s;
        double keepAlivePeriod @unit(s) = 210s;
        double rpKeepAlivePeriod @unit(s) = 3 * registerSuppressionTime + registerProbeTime;
        double registerSuppressionTime @unit(s) = 60s;
        double registerProbeTime @unit(s) = 5s;
        double assertTime @unit(s) = 180s;
        double assertOverrideInterval @unit(s) = 3s;

        @signal[sentHelloPk](type=cPacket);
        @signal[rcvdHelloPk](type=cPacket);
        @signal[sentRegisterPk](type=cPacket);
        @signal[rcvdRegisterPk](type=cPacket);
        @signal[sentRegisterStopPk](type=cPacket);
        @signal[rcvdRegisterStopPk](type=cPacket);
        @signal[sentJoinPrunePk](type=cPacket);
        @signal[rcvdJoinPrunePk](type=cPacket);
        @signal[sentAssertPk](type=cPacket);
        @signal[rcvdAssertPk](type=cPacket);

    gates:
        input ipIn;
        output ipOut;
}
