package inet.protocoltest.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import inet.node.ethernet.EthernetLink;
import inet.protocoltest.PacketTap;

network DtlsPathMtuNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: StandardHost;
        router: Router;
        host2: StandardHost;
        tap: PacketTap;
        peerTap: PacketTap;
    connections:
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> tap.a;
        tap.b <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> router.ethg++;
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 2ms; } <--> router.ethg++;
        router.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> peerTap.a;
        peerTap.b <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> host2.ethg++;
}
