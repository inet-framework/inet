// Cumtom module with these commands:
//  <joinMulticastGroup at="1s" module="XXX" node="host" ifname="ppp0" group="225.0.0.1"/>
//  <leaveMulticastGroup at="2s" module="XXX" node="host" ifname="ppp0" group="225.0.0.1"/>
//  <checkJoinedMulticastGroups at="1.5s" module="" node="host" ifname="ppp0" group="224.0.0.1 225.0.0.1"/>
//  <checkReportedMulticastGroups at="1.5s" module="" node="router" ifname="ppp0" group="225.0.0.1 225.0.0.2"/>

simple IGMPTester
{
    parameters:
        string outputFile = default("");
}
