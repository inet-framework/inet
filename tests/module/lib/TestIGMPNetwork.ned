import inet.base.NotificationBoard;
import inet.networklayer.autorouting.ipv4.IPv4NodeConfigurator;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IIGMP;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.world.scenario.ScenarioManager;


network TestIGMPNetwork
{
    types:
        module TestIGMPNode
        {
            parameters:
                @node;
                string igmpType = default("IGMPv3");

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
                notificationBoard: NotificationBoard {
                    @display("p=285,31");
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
