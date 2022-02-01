//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.crosstalk;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network Crosstalk
{
    parameters:
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,300;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        aNode[2]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=400,200");
        }
        bNode[2]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=250,200");
        }
}
