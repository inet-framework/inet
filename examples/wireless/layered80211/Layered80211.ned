//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.layered80211;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.bitlevel.Ieee80211LayeredDimensionalRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network Layered80211
{
    parameters:
        int numHosts;
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,300;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211LayeredDimensionalRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=250,200");
        }
}
