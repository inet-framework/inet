package inet.protocoltest.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.node.ethernet.EthernetLink;

network DtlsMigrationRoutedNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: StandardHost;
        router: Router;
        host2: StandardHost;
    connections:
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> router.ethg++;
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> router.ethg++;
        router.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> host2.ethg++;
}
