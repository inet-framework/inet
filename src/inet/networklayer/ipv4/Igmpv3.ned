//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.networklayer.ipv4;

import inet.common.SimpleModule;

//
// Implementation of Internet Group Management Protocol version 3 (IGMPv3)
// as specified in RFC 3376. Manages multicast group memberships for IPv4
// networks, supporting source-specific multicast. Handles both host-side
// functionality (joining/leaving groups) and router-side functionality
// (querying hosts and tracking memberships).
//
simple Igmpv3 extends SimpleModule like IIgmp
{
    parameters:
        @class(Igmpv3);
        bool enabled = default(true);
        string interfaceTableModule;
        string routingTableModule;
        string checksumMode @enum("declared","computed") = default("declared");
        int    robustnessVariable = default(2); // IGMP is robust to (robustnessVariable-1) packet loss
        double queryInterval @unit(s) = default(125s);
        double queryResponseInterval @unit(s) = default(10s);
        double groupMembershipInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);
        double otherQuerierPresentInterval @unit(s) = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));
        double startupQueryInterval @unit(s) = default(queryInterval / 4);
        int    startupQueryCount = default(robustnessVariable);
        double lastMemberQueryInterval @unit(s) = default(1s);
        int    lastMemberQueryCount = default(robustnessVariable);
        double unsolicitedReportInterval @unit(s) = default(10s);
        //double version1RouterPresentInterval @unit(s) = default(400s);
        @display("i=block/cogwheel");
        @selfMessageKinds(inet::IgmpTimerKind);

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);  // Delivered IGMP packets
        output ipOut @labels(Ipv4ControlInfo/down); // To ~Ipv4

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

