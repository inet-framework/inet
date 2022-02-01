//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network ComplexConfiguratorNetwork
{
    parameters:
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,400;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMediumA: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        radioMediumB: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,300;is=s");
        }
        source: AdhocHost {
            @display("p=50,271;i=device/laptop");
        }
        wirelessA1: AdhocHost {
            @display("p=201,230");
        }
        wirelessA2: AdhocHost {
            @display("p=334,304");
        }
        wiredA1: Router {
            @display("p=107,458");
        }
        wiredA2: Router {
            @display("p=398,458");
        }
        shared1: AdhocHost {
            @display("p=481,271;i=device/laptop");
        }
        shared2: AdhocHost {
            @display("p=674,271;i=device/laptop");
        }
        wirelessB1: AdhocHost {
            @display("p=822,197");
        }
        wirelessB2: AdhocHost {
            @display("p=925,307");
        }
        wiredB1: Router {
            @display("p=737,82");
        }
        wiredB2: Router {
            @display("p=977,82");
        }
        destination: AdhocHost {
            @display("p=1069,271;i=device/laptop");
        }
    connections:
        source.ethg++ <--> Eth1G <--> wiredA1.ethg++;
        wiredA1.ethg++ <--> Eth1G <--> wiredA2.ethg++;
        wiredA2.ethg++ <--> Eth1G <--> shared1.ethg++;
        shared1.ethg++ <--> Eth1G <--> shared2.ethg++;
        shared2.ethg++ <--> Eth1G <--> wiredB1.ethg++;
        wiredB1.ethg++ <--> Eth1G <--> wiredB2.ethg++;
        wiredB2.ethg++ <--> Eth1G <--> destination.ethg++;
}
