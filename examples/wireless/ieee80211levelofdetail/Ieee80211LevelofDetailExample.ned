//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.wireless.ieee80211levelofdetail;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network Ieee80211LevelofDetailShowcase
{
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,250");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: <> like IRadioMedium {
            parameters:
                @display("p=100,50");
        }
        sourceHost: AdhocHost {
            parameters:
                @display("p=200,100");
        }
        destinationHost: AdhocHost {
            parameters:
                @display("p=400,100");
        }
}
