package inet.showcases.general.parsim;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
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

module Subnetwork
{
    parameters:
        @display("i=misc/cloud");
        **.radioMediumModule = absPath(".radioMedium");
        **.networkConfiguratorModule = absPath(".configurator");
    gates:
        inout ethg[];
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=300,50;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=300,100;is=s");
        }
        wiredHost1: StandardHost {
            @display("p=50,150");
        }
        wiredHost2: StandardHost {
            @display("p=50,250");
        }
        wiredHost3: StandardHost {
            @display("p=150,100");
        }
        wiredHost4: StandardHost {
            @display("p=150,300");
        }
        switch: EtherSwitch {
            @display("p=150,200");
        }
        wirelessHost1: WirelessHost {
            @display("p=350,150");
        }
        wirelessHost2: WirelessHost {
            @display("p=350,250");
        }
        wirelessHost3: WirelessHost {
            @display("p=450,100");
        }
        wirelessHost4: WirelessHost {
            @display("p=450,300");
        }
        ap: AccessPoint {
            @display("p=450,200");
        }
    connections:
        wiredHost1.ethg++ <--> Ethernet100 <--> switch.ethg++;
        wiredHost2.ethg++ <--> Ethernet100 <--> switch.ethg++;
        wiredHost3.ethg++ <--> Ethernet100 <--> switch.ethg++;
        wiredHost4.ethg++ <--> Ethernet100 <--> switch.ethg++;
        switch.ethg++ <--> Ethernet100 <--> ap.ethg++;
        switch.ethg++ <--> ethg++;
}

network ParsimNetwork
{
    parameters:
        @display("bgb=812,600");
    submodules:
        visualizer: <default("")> like IIntegratedVisualizer if typename != "" {
            @display("p=750,100;is=s");
        }
        routerA: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkA.configurator");
            @display("p=200,100");
        }
        routerB: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkB.configurator");
            @display("p=200,300");
        }
        routerC: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkC.configurator");
            @display("p=400,300");
        }
        routerD: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkD.configurator");
            @display("p=400,100");
        }
        subnetworkA: Subnetwork {
            @display("p=50,50");
        }
        subnetworkB: Subnetwork {
            @display("p=50,350");
        }
        subnetworkD: Subnetwork {
            @display("p=500,50");
        }
        subnetworkC: Subnetwork {
            @display("p=500,350");
        }
    connections:
        routerA.pppg++ <--> PppChannel <--> routerD.pppg++;
        routerD.pppg++ <--> PppChannel <--> routerC.pppg++;
        routerC.pppg++ <--> PppChannel <--> routerB.pppg++;
        routerB.pppg++ <--> PppChannel <--> routerA.pppg++;
        subnetworkA.ethg++ <--> Ethernet100 <--> routerA.ethg++;
        subnetworkB.ethg++ <--> Ethernet100 <--> routerB.ethg++;
        subnetworkD.ethg++ <--> Ethernet100 <--> routerD.ethg++;
        subnetworkC.ethg++ <--> Ethernet100 <--> routerC.ethg++;
}

