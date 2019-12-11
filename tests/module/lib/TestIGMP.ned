package inet.test.moduletest.lib;

import inet.networklayer.ipv4.IIgmp;

simple TestIGMP like IIgmp
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        string crcMode @enum("declared","computed") = default("declared");
        bool enabled = default(true);
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
        string outputFile = default("");

        @display("i=block/cogwheel_s");

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(Ipv4ControlInfo/down); // to ~Ipv4

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

