//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.errorrate;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network ErrorRateShowcase
{
    submodules:
        visualizer: IntegratedCanvasVisualizer {
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
        sourceHost: AdhocHost {
            parameters:
                @display("p=200,100");
        }
        destinationHost: AdhocHost {
            parameters:
                @display("p=400,100");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=252,250");
        }
}
