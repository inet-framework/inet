package inet.tests.ethernet;

import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.EthernetHub;
import ned.DatarateChannel;

network HostsWithHub
{
    submodules:
        hostA: SimpleEthTrafGenHost {
            parameters:
                @display("p=80,200");
        }
        hostB: SimpleEthTrafGenHost {
            parameters:
                @display("p=200,200");
        }
        hostC: SimpleEthTrafGenHost {
            parameters:
                @display("p=320,200");
        }
        hub: EthernetHub {
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
