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
    delay = 0.1us; // 20 meters
}

channel PppChannel extends ned.DatarateChannel
{
    datarate = 1Gbps;
    delay = 10us; // 2000 meters
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
            @display("p=243,38;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=349.11246,39.053257;is=s");
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
        @display("bgb=700,427");
    submodules:
        //        visualizer: <default("")> like IIntegratedVisualizer if typename != "" {
        //            @display("p=633,50;is=s");
        //        }
        routerA: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkA.configurator");
            @display("p=250,100");
        }
        routerB: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkB.configurator");
            @display("p=250,300");
        }
        routerC: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkC.configurator");
            @display("p=450,300");
        }
        routerD: Router {
            **.networkConfiguratorModule = absPath("^.subnetworkD.configurator");
            @display("p=450,100");
        }
        subnetworkA: Subnetwork {
            @display("p=100,50");
        }
        subnetworkB: Subnetwork {
            @display("p=100,350");
        }
        subnetworkD: Subnetwork {
            @display("p=600,50");
        }
        subnetworkC: Subnetwork {
            @display("p=600,350");
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

