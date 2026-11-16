package inet.protocoltest.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetLink;
import inet.protocoltest.PacketTap;

// Two isolated Ethernet links terminate on distinct interfaces of one server.
network DtlsMultihomedNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: StandardHost;
        host2: StandardHost;
        host3: StandardHost;
        tap: PacketTap;
        tap3: PacketTap;
    connections:
        host1.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> tap.a;
        tap.b <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> host2.ethg++;
        host3.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> tap3.a;
        tap3.b <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> host2.ethg++;
}
