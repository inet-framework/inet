//
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


package inet.networklayer.ipv4;

//
// Imlementation of IGMPv2 protocol.
//
// This module implements both IGMPv2 host and router logic.
//
// Host parameters:
//  - enabled:
//  - unsolicitedReportInterval
//
// Router parameters:
//  - enabled:
//  - externalRouter:
//  - queryInterval: 
//  - queryResponseInterval
//  - groupMembershipInterval
//  - otherQuerierPresentInterval
//  - startupQueryInterval
//  - startupQueryCount
//  - lastMemberQueryInterval
//  - lastMemberQuery
//  - roboustnessVariable
//
// Public interface:
//  configureInterface(ie)
//  joinMulticastGroup(ie, groupAddr)
//  leaveMulticastGroup(ie, groupAddr)
simple IGMP
{
    parameters:
        bool enableLogging = default(false);
        bool enabled = default(true);
        bool externalRouter = default(false);
        int    robustnessVariable = default(2);
        double queryInterval @unit("s") = default(125s);
        double queryResponseInterval @unit("s") = default(10s);
        double groupMembershipInterval @unit("s") = default((robustnessVariable * queryInterval) + queryResponseInterval);
        double otherQuerierPresentInterval @unit("s") = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));
        double startupQueryInterval @unit("s") = default(queryInterval / 4);
        int    startupQueryCount = default(robustnessVariable);
        double lastMemberQueryInterval @unit("s") = default(1s);
        int    lastMemberQueryCount = default(robustnessVariable);
        double unsolicitedReportInterval @unit("s") = default(10s);
//      double version1RouterPresentInterval @unit("s") = default(400s);
        @display("i=block/cogwheel_s");

    gates:
        input ipIn @labels(IPv4ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(IPv4ControlInfo/down); // to ~IPv4

        input routerIn @labels(IPv4ControlInfo/up);
        output routerOut @labels(IPv4ControlInfo/down);
}

