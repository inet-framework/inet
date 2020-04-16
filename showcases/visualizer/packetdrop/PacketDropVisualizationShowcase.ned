package inet.showcases.visualizer.packetdrop;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100G;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import ned.DelayChannel;


network PacketDropQueueOverflowShowcase
{
    @display("bgb=720,400");
    submodules:
        source: StandardHost {
            @display("p=80,198");
        }
        etherSwitch: EtherSwitch {
            @display("p=211,197");
        }
        router: Router {
            @display("p=323,198");
        }
        accessPoint: AccessPoint {
            @display("p=449,197");
        }
        destination: WirelessHost {
            @display("p=589,194");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=88,37");
        }
        visualizer: IntegratedVisualizer {
            @display("p=267,37");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=465,37");
        }
    connections:
        router.ethg++ <--> Eth100M <--> accessPoint.ethg++;
        source.ethg++ <--> Eth100G <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> Eth10M <--> router.ethg++;
}

network PacketDropArpResolutionFailedShowcase
{
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        source: AdhocHost {
            parameters:
                @display("p=250,150");
        }
}

network PacketDropMacRetryLimitReachedShowcase
{
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        source: AdhocHost {
            parameters:
                @display("p=250,150");
        }
        destination: AdhocHost {
            parameters:
                @display("p=400,150");
        }
}

network PacketDropInterfaceNotConnectedShowcase
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,40");
        }
        visualizer: IntegratedVisualizer {
            @display("p=60,101");
        }
        host1: StandardHost {
            @display("p=110,223");
        }
        host2: StandardHost {
            @display("p=269,223");
        }
    connections:
        host1.ethg++ <--> Eth100M {  disabled = true;@display("t=disabled"); } <--> host2.ethg++;
}

network PacketDropNoRouteToDestinationShowcase
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,40");
        }
        visualizer: IntegratedVisualizer {
            @display("p=60,101");
        }
        host1: StandardHost {
            @display("p=110,223");
        }
        host2: StandardHost {
            @display("p=269,223");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}


