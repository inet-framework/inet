//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.networkpathactivity;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.aodv.AodvRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;
import inet.visualizer.common.NetworkRouteVisualizer;

network NetworkPathSimpleShowcase
{
    @display("bgb=600,250");
    submodules:
        pathVisualizer: NetworkRouteVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        source: StandardHost {
            @display("p=200,125");
        }
        destination: StandardHost {
            @display("p=500,125");
        }

    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

network NetworkPathComplexShowcase
{
    @display("bgb=1050,720");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        source1: StandardHost {
            @display("p=250,50");
        }
        source2: StandardHost {
            @display("p=947.52496,110.439995");
        }
        destination1: StandardHost {
            @display("p=749.235,611.185");
        }
        destination2: StandardHost {
            @display("p=131.856,533.856");
        }
        host1: StandardHost {
            @display("p=379.488,49.848");
        }
        videoClient: StandardHost {
            @display("p=831.336,109.344");
        }
        host2: StandardHost {
            @display("p=894.048,533.856");
        }
        videoServer: StandardHost {
            @display("p=249.24,639.984");
        }
        etherSwitch0: EthernetSwitch {
            @display("p=249.24,160.8");
        }
        etherSwitch1: EthernetSwitch {
            @display("p=947.52496,229.665");
        }
        etherSwitch2: EthernetSwitch {
            @display("p=279.865,515.805");
        }
        etherSwitch3: EthernetSwitch {
            @display("p=731.665,448.03497");
        }
        router1: Router {
            @display("p=749.235,230.92");
        }
        router2: Router {
            @display("p=249.745,346.38");
        }
        router3: Router {
            @display("p=461.496,448.632");
        }
        router4: Router {
            @display("p=575.664,313.56");
        }
        router0: Router {
            @display("p=484.008,180.09601");
        }
    connections:
        source1.ethg++ <--> Eth100M <--> etherSwitch0.ethg++;
        etherSwitch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> destination1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> router1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> source2.ethg++;
        router2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        destination2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        router2.pppg++ <--> Eth100M <--> router3.pppg++;
        etherSwitch0.ethg++ <--> Eth100M <--> host1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> videoClient.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> host2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> videoServer.ethg++;
        router1.pppg++ <--> Eth100M <--> router0.pppg++;
        router0.pppg++ <--> Eth100M <--> router2.pppg++;
        router4.pppg++ <--> Eth100M <--> router3.pppg++;
        router4.pppg++ <--> Eth100M <--> router2.pppg++;
        router4.pppg++ <--> Eth100M <--> router1.pppg++;
        router0.pppg++ <--> Eth100M <--> router4.pppg++;
}

network NetworkPathMobileShowcase
{
    @display("bgb=660,630");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,250");
        }
        source: AodvRouter {
            @display("p=229,46");
        }
        destination: AodvRouter {
            @display("p=488.832,512.952");
        }
        aodvRouter1: AodvRouter {
            @display("p=421.296,295.872");
        }
        aodvRouter2: AodvRouter {
            @display("p=273.36002,440.592");
        }
        aodvRouter3: AodvRouter {
            @display("p=305.52,188.136");
        }
        aodvRouter4: AodvRouter {
            @display("p=562.8,328.032");
        }
        aodvRouter5: AodvRouter {
            @display("p=421.296,77.184");
        }
}

network NetworkPathRIPShowcase
{
    @display("bgb=1100,680");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,350");
        }
        wiredSource: StandardHost {
            @display("p=257.28,535.464");
        }
        wiredDestination: StandardHost {
            @display("p=802.392,149.544");
        }
        wirelessSource: WirelessHost {
            @display("p=406.824,80.4");
        }
        wirelessDestination: WirelessHost {
            @display("p=786.312,567.624");
        }
        host1: StandardHost {
            @display("p=445.41602,535.464");
        }
        host2: StandardHost {
            @display("p=993.744,149.544");
        }
        accessPoint0: AccessPoint {
            @display("p=319.992,154.368");
        }
        accessPoint1: AccessPoint {
            @display("p=876.36,479.184");
        }
        etherSwitch0: EthernetSwitch {
            @display("p=876.36,276.576");
        }
        etherSwitch1: EthernetSwitch {
            @display("p=329.64,402");
        }
        router0: Router {
            @display("p=383.73126,279.6875");
        }
        router1: Router {
            @display("p=633.552,192.96");
        }
        router2: Router {
            @display("p=787.60004,404.98752");
        }
        router3: Router {
            @display("p=554.76,458.28");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        router0.pppg++ <--> Eth100M <--> router3.pppg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.pppg++ <--> Eth100M <--> router3.pppg++;
        router2.ethg++ <--> Eth100M <--> etherSwitch0.ethg++;
        etherSwitch0.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
        router2.ethg++ <--> Eth100M <--> accessPoint1.ethg++;
        router0.ethg++ <--> Eth100M <--> etherSwitch1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> wiredSource.ethg++;
        router0.ethg++ <--> Eth100M <--> accessPoint0.ethg++;
        host1.ethg++ <--> Eth100M <--> etherSwitch1.ethg++;
        host2.ethg++ <--> Eth100M <--> etherSwitch0.ethg++;
}

