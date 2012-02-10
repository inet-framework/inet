package inet.tests.ethernet;

import inet.linklayer.ethernet.EtherBus;
import inet.nodes.ethernet.EtherHost2;
import inet.world.scenario.ScenarioManager;
import ned.DatarateChannel;

//
// Sample Ethernet LAN: hosts connected via bus
//

network HostsWithBus
{
    types:
    submodules:
        hostA: EtherHost2 {
            parameters:
                @display("p=72,80");
        }
        hostB: EtherHost2 {
            parameters:
                @display("p=334,80");
        }
        hostC: EtherHost2 {
            parameters:
                @display("p=334,80");
        }
        bus: EtherBus {
            parameters:
                positions = "0 5 10";
                propagationSpeed = 2e8 mps; // 1us = 200m
                @display("p=250,180;b=400,6;o=#408060");
            gates:
                ethg[3];
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<empty/>"));
                @display("p=184,180");
        }
    connections:
        bus.ethg++ <--> DatarateChannel <--> hostA.ethg;
        bus.ethg++ <--> DatarateChannel <--> hostB.ethg;
        bus.ethg++ <--> DatarateChannel <--> hostC.ethg;
}

