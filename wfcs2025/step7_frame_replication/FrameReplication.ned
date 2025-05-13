
package step7_frame_replication;

import Wfcs2025NetworkBase;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network FrameReplicationShowcase extends Wfcs2025NetworkBase
{
    submodules:
        failureProtectionConfigurator: <default("")> like INetworkConfigurator if typename != "" {
            @display("p=200,400;is=s");
        }
        streamRedundancyConfigurator: <default("")> like INetworkConfigurator if typename != "" {
            @display("p=200,500;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=200,600;is=s");
        }
        source: TsnDevice {
            @display("p=450,350");
        }
        s1: LocalTsnSwitch {
            @display("p=700,350");
        }
        s2a: LocalTsnSwitch {
            @display("p=950,200");
        }
        s3a: LocalTsnSwitch {
            @display("p=1200,200");
        }
        s2b: LocalTsnSwitch {
            @display("p=950,500");
        }
        s3b: LocalTsnSwitch {
            @display("p=1200,500");
        }
        destination: TsnDevice {
            @display("p=1450,350");
        }
    connections:
        s3a.ethg++ <--> EthernetLink <--> destination.ethg++;
        s3b.ethg++ <--> EthernetLink <--> destination.ethg++;
        s2a.ethg++ <--> EthernetLink <--> s3a.ethg++;
        s2b.ethg++ <--> EthernetLink <--> s3b.ethg++;
        s2a.ethg++ <--> EthernetLink <--> s2b.ethg++;
        s1.ethg++ <--> EthernetLink <--> s2a.ethg++;
        s1.ethg++ <--> EthernetLink <--> s2b.ethg++;
        source.ethg++ <--> EthernetLink <--> s1.ethg++;
}

module LocalTsnSwitch extends TsnSwitch
{
    @defaultStatistic("gateStateChanged:vector"; module="eth[0].macLayer.queue.gate[0]");
}

