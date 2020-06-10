package inet.tests.ethernet;

import inet.linklayer.ethernet.EtherBus;
import inet.node.ethernet.EtherHost2;
import inet.common.scenario.ScenarioManager;
import ned.TransmissionChannel;

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
        bus.ethg++ <--> TransmissionChannel <--> hostA.ethg;
        bus.ethg++ <--> TransmissionChannel <--> hostB.ethg;
        bus.ethg++ <--> TransmissionChannel <--> hostC.ethg;
}

