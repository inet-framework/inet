package inet.protocoltest.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.EthernetLink;

network DtlsMigrationNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: StandardHost;
        sw: EthernetSwitch;
        host2: StandardHost;
    connections:
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> sw.ethg++;
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> sw.ethg++;
        host2.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> sw.ethg++;
}
