import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.ipv4.IIGMP;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.common.scenario.ScenarioManager;


network TestIGMPNetwork
{
    types:
        module TestIGMPNode
        {
            parameters:
                @networkNode;
                string igmpType = default("IGMPv3");
                *.interfaceTableModule = absPath(".interfaceTable");
                *.routingTableModule = absPath(".routingTable");
            submodules:
                interfaceTable: InterfaceTable {
                    @display("p=57,31");
                }
                routingTable: IPv4RoutingTable {
                    @display("p=168,31");
                }
                tester: IGMPTester {
                    @display("p=70,153");
                }
                igmp: <igmpType> like IIGMP {
                    @display("p=176,153");
                }
                nodeConfigurator: IPv4NodeConfigurator {
                    @display("p=285,95");
                    networkConfiguratorModule = "";
                    configureRoutingTable = false;
                }
            connections allowunconnected:
                tester.igmpOut --> igmp.ipIn;
                igmp.ipOut --> tester.igmpIn;
        }
    submodules:
        node: TestIGMPNode;
        scenarioManager: ScenarioManager {
            @display("p=139,19");
        }
}
