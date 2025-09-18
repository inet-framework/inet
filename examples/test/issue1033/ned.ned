

package inet.examples.test.issue1033;

import inet.common.scenario.ScenarioManager;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


network netmask_bug extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(1Gbps);
    submodules:
        scenarioManager: ScenarioManager;

        Node1: TsnDevice {
            gates:
                ethg[1];
        }

        Switch1: TsnSwitch {
            gates:
                ethg[2];
        }

        Node2: TsnDevice {
            gates:
                ethg[1];
        }

    connections:
        Switch1.ethg[0] <--> EthernetLink {  datarate = 1Gbps; } <--> Node1.ethg[0];
        Switch1.ethg[1] <--> EthernetLink {  datarate = 1Gbps; } <--> Node2.ethg[0];
}
