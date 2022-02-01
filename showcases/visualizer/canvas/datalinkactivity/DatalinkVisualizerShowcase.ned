//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.datalinkactivity;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.aodv.AodvRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.DataLinkVisualizer;
import inet.visualizer.common.IntegratedMultiVisualizer;
import inet.visualizer.common.IntegratedVisualizer;

network DataLinkVisualizerEnablingWiredShowcase
{
    @display("bgb=700,250");
    submodules:
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        wiredSource: StandardHost {
            @display("p=300,110");
        }
        wiredDestination: StandardHost {
            @display("p=600,110");
        }
    connections:
        wiredSource.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
}


network DataLinkVisualizerEnablingWirelessShowcase
{
    @display("bgb=700,250");
    submodules:
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,120");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,190");
        }
        wirelessSource: AdhocHost {
            @display("p=300,110");
        }
        wirelessDestination: AdhocHost {
            @display("p=600,110");
        }
}

network DataLinkVisualizerActivityLevelShowcase
{
    @display("bgb=700,250");
    submodules:
        visualizer: IntegratedMultiVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,120");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,190");
        }
        person1: AdhocHost {
            @display("p=269,42;i=misc/person3");
        }
        videoServer: AdhocHost {
            @display("p=636,41;i=misc/desktoppc2");
        }
        person2: AdhocHost {
            @display("p=452,180;i=misc/person4");
        }
}

network DataLinkVisualizerFilteringShowcase
{
    @display("bgb=900,600");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=80,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=80,120");
        }
        etherSwitch1: EthernetSwitch {
            @display("p=376.896,169.12");
        }
        etherSwitch3: EthernetSwitch {
            @display("p=723.592,200.528");
        }
        source1: StandardHost {
            @display("p=206.568,50.736");
        }
        etherSwitch4: EthernetSwitch {
            @display("p=484.408,283.88");
        }
        destination1: StandardHost {
            @display("p=556.888,485.616");
        }
        destination2: StandardHost {
            @display("p=798.488,343.072");
        }
        source2: StandardHost {
            @display("p=173.952,227.104");
        }
        etherSwitch2: EthernetSwitch {
            @display("p=329.784,428.84");
        }
        host1: StandardHost {
            @display("p=233.144,506.152");
        }
        host2: StandardHost {
            @display("p=822.648,135.296");
        }
    connections:
        etherSwitch1.ethg++ <--> Eth100M <--> source1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> etherSwitch4.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> source2.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> destination2.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> destination1.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> host2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> host1.ethg++;
}

network DataLinkVisualizerMobileShowcase
{
    @display("bgb=620,500");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=70,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=70,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=70,250");
        }
        destination: AodvRouter {
            @display("p=512.272,249.796");
        }
        source: AodvRouter {
            @display("p=187.664,207.952");
        }
        aodvRouter1: AodvRouter {
            @display("p=201.612,311.928");
        }
        aodvRouter2: AodvRouter {
            @display("p=465.356,158.5");
        }
        aodvRouter3: AodvRouter {
            @display("p=426.048,311.928");
        }
        aodvRouter4: AodvRouter {
            @display("p=400.688,88.76");
        }
        aodvRouter5: AodvRouter {
            @display("p=333.484,177.52");
        }
        aodvRouter6: AodvRouter {
            @display("p=323.34,262.476");
        }
        aodvRouter7: AodvRouter {
            @display("p=265.012,109.048");
        }
}

