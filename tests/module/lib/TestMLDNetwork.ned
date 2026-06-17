package inet.test.moduletest.lib;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.icmpv6.IMld;
import inet.networklayer.ipv6.Ipv6RoutingTable;

//
// Tester NIC that wraps the scriptable ~TestMld module, the IPv6/MLD counterpart of
// IgmpTesterNic. It is the single multicast interface ("eth0") of a TestMLDNode and
// is wired to the node's MLD module's ipIn/ipOut gates.
//
module MldTesterNic
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = absPath(this.interfaceTableModule);
        *.routingTableModule = absPath(this.routingTableModule);
        @class(NetworkInterface);
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        mac: TestMld {
            @display("p=70,153");
        }
    connections:
        upperLayerIn --> mac.upperLayerIn;
        upperLayerOut <-- mac.upperLayerOut;
}

//
// IPv6/MLD counterpart of TestIGMPNetwork: a single node holding an InterfaceTable, an
// Ipv6RoutingTable, the scriptable tester NIC, and an MLD module (Mldv2 by default),
// plus a ScenarioManager driving the test script.
//
network TestMLDNetwork
{
    types:
        module TestMLDNode
        {
            parameters:
                @networkNode;
                *.interfaceTableModule = absPath(".interfaceTable");
                *.routingTableModule = absPath(".routingTable");
            submodules:
                interfaceTable: InterfaceTable {
                    @display("p=57,31");
                }
                routingTable: Ipv6RoutingTable {
                    @display("p=168,31");
                }
                tester: MldTesterNic {
                    @display("p=70,153");
                }
                mld: <default("Mldv2")> like IMld {
                    @display("p=176,153");
                }
            connections allowunconnected:
                tester.upperLayerOut --> mld.ipIn;
                mld.ipOut --> tester.upperLayerIn;
        }
    submodules:
        node: TestMLDNode;
        scenarioManager: ScenarioManager {
            @display("p=139,19");
        }
}
