//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.ratecontrol;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network RateControlShowcase
{
    parameters:
        @display("b=297,203;bgb=1300,460");
    submodules:
        sourceHost: WirelessHost {
            parameters:
                @display("r=,,#707070");
        }
        sinkHost: WirelessHost {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=131.53847,86.15385");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=130.76924,168.46155");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=131.53847,253.84616");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=131.53847,336.15387");
        }
}

