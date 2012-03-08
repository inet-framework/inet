import inet.networklayer.ipv4.IIGMP;

simple TestIGMP like IIGMP
{
    parameters:
        bool enabled = default(true);
        int    robustnessVariable = default(2); // IGMP is roboust to (roboustnessVariable-1) packet loss
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
        string outputFile = default("");

        @display("i=block/cogwheel_s");

    gates:
        input ipIn @labels(IPv4ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(IPv4ControlInfo/down); // to ~IPv4

        input routerIn @labels(IPv4ControlInfo/up);
        output routerOut @labels(IPv4ControlInfo/down);
}

