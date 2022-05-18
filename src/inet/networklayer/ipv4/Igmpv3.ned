//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4;

simple Igmpv3 like IIgmp
{
    parameters:
        bool enabled = default(true);
        string interfaceTableModule;
        string routingTableModule;
        string crcMode @enum("declared","computed") = default("declared");
        int    robustnessVariable = default(2); // IGMP is roboust to (roboustnessVariable-1) packet loss
        double queryInterval @unit(s) = default(125s);
        double queryResponseInterval @unit(s) = default(10s);
        double groupMembershipInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);
        double otherQuerierPresentInterval @unit(s) = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));
        double startupQueryInterval @unit(s) = default(queryInterval / 4);
        int    startupQueryCount = default(robustnessVariable);
        double lastMemberQueryInterval @unit(s) = default(1s);
        int    lastMemberQueryCount = default(robustnessVariable);
        double unsolicitedReportInterval @unit(s) = default(10s);
//      double version1RouterPresentInterval @unit(s) = default(400s);
        @display("i=block/cogwheel");
        @selfMessageKinds(inet::IgmpTimerKind);

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(Ipv4ControlInfo/down); // to ~Ipv4

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

