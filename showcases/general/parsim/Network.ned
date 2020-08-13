package inet.showcases.general.parsim;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


channel Ethernet100 extends ned.DatarateChannel
{
    datarate = 100Mbps;
    delay = 0.1us;
}

channel PppChannel extends ned.DatarateChannel
{
    datarate = 1Gbps;
    delay = 1us;
}

network ParSimNetworkBackbone
{
    @display("bgb=812,400");
    submodules:
        hostA1: StandardHost {
            @display("p=56,44");
        }
        hostA2: StandardHost {
            @display("p=56,147");
        }
        switchA: EtherSwitch {
            @display("p=130,97");
        }
        hostB1: WirelessHost {
            @display("p=56,232");
        }
        hostB2: WirelessHost {
            @display("p=56,335");
        }
        APB: AccessPoint {
            @display("p=130,285");
        }
        routerB: Router {
            @display("p=221.34,271.32");
        }
        hostC1: WirelessHost {
            @display("p=585,45");
        }
        hostC2: WirelessHost {
            @display("p=585,154");
        }
        HostD1: StandardHost {
            @display("p=585,236");
        }
        HostD2: StandardHost {
            @display("p=591,325");
        }
        APC: AccessPoint {
            @display("p=515,97");
        }
        swtichD: EtherSwitch {
            @display("p=503,280");
        }
        configuratorA: Ipv4NetworkConfigurator {
            @display("p=736,47");
        }
        configuratorB: Ipv4NetworkConfigurator {
            @display("p=736,147");
        }
        configuratorC: Ipv4NetworkConfigurator {
            @display("p=736,247");
        }
        configuratorD: Ipv4NetworkConfigurator {
            @display("p=736,347");
        }
        radioMedium2: Ieee80211ScalarRadioMedium {
            @display("p=467,46");
        }
        radioMedium1: Ieee80211ScalarRadioMedium {
            @display("p=193,340");
        }
        routerA: Router {
            @display("p=222,112");
        }
        routerC: Router {
            @display("p=371,112");
        }
        routerD: Router {
            @display("p=370.26,271.32");
        }
    connections:
        hostA1.ethg++ <--> Ethernet100 <--> switchA.ethg++;
        hostA2.ethg++ <--> Ethernet100 <--> switchA.ethg++;
        swtichD.ethg++ <--> Ethernet100 <--> HostD1.ethg++;
        swtichD.ethg++ <--> Ethernet100 <--> HostD2.ethg++;
        routerA.pppg++ <--> PppChannel <--> routerC.pppg++;
        routerC.pppg++ <--> PppChannel <--> routerD.pppg++;
        routerD.pppg++ <--> PppChannel <--> routerB.pppg++;
        routerB.pppg++ <--> PppChannel <--> routerA.pppg++;
        switchA.ethg++ <--> Ethernet100 <--> routerA.ethg++;
        routerC.ethg++ <--> Ethernet100 <--> APC.ethg++;
        APB.ethg++ <--> Ethernet100 <--> routerB.ethg++;
        routerD.ethg++ <--> Ethernet100 <--> swtichD.ethg++;
}

network DebugNetwork extends ParSimNetworkBackbone
{
    submodules:
        visualizer: IntegratedVisualizer;
}

network MoreHostsNetwork extends ParSimNetworkBackbone
{
    submodules:
        hostA3: StandardHost {
            @display("p=133.62,25.5");
        }
        hostA4: StandardHost {
            @display("p=123.42,161.16");
        }
        hostD4: StandardHost {
            @display("p=480.42,363.12");
        }
        hostD3: StandardHost {
            @display("p=475.32,192.78");
        }
        hostB3: WirelessHost {
            @display("p=137.7,224.4");
        }
        hostB4: WirelessHost {
            @display("p=106.08,350.88");
        }
        hostC3: WirelessHost {
            @display("p=530.4,25.5");
        }
        hostC4: WirelessHost {
            @display("p=519.18,158.1");
        }
    connections:
        switchA.ethg++ <--> Ethernet100 <--> hostA3.ethg++;
        switchA.ethg++ <--> Ethernet100 <--> hostA4.ethg++;
        swtichD.ethg++ <--> Ethernet100 <--> hostD3.ethg++;
        swtichD.ethg++ <--> Ethernet100 <--> hostD4.ethg++;
}

network DebugMoreHostsNetwork extends MoreHostsNetwork
{
    submodules:
        visualizer: IntegratedVisualizer;
}

