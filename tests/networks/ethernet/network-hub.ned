package inet.tests.ethernet;

import inet.linklayer.ethernet.EtherHub;
import inet.node.ethernet.EtherHost2;
import inet.common.scenario.ScenarioManager;
import ned.DatarateChannel;


network HostsWithHub
{
    submodules:
        hostA: EtherHost2 {
            parameters:
                @display("p=80,200");
        }
        hostB: EtherHost2 {
            parameters:
                @display("p=200,200");
        }
        hostC: EtherHost2 {
            parameters:
                @display("p=320,200");
        }
        hub: EtherHub {
            parameters:
                @display("p=200,80");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<empty/>"));
                @display("p=80,40");
        }
    connections:
        hub.ethg++ <--> DatarateChannel <--> hostA.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostB.ethg;
        hub.ethg++ <--> DatarateChannel <--> hostC.ethg;
}
