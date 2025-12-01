package inet.showcases.tsn.trafficshaping.timeawareshaper;

import inet.common.Module;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;
import inet.visualizer.contract.IIntegratedVisualizer;


network TimeAwareShapingShowcaseNetwork extends Module
{
    parameters:
        bool hasGlobalArp = default(true);
        *.ipv4.arp.typename = default(hasGlobalArp ? "GlobalArp" : "Arp");
    submodules:
        client1: TsnDevice {
            @display("p=500,100");
        }
        client2: TsnDevice {
            @display("p=500,300");
        }
        switch: TsnSwitch {
            @display("p=800,200");
        }
        server: TsnDevice {
            @display("p=1100,200");
        }
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=200,100;is=s");
        }
        configurator: <default("Ipv4NetworkConfigurator")> like IL3NetworkConfigurator if typename != "" {
            @display("p=200,200;is=s");
        }
        macForwardingTableConfigurator: <default(hasGlobalArp ? "MacForwardingTableConfigurator" : "")> like INetworkConfigurator if typename != "" {
            @display("p=200,300;is=s");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}

