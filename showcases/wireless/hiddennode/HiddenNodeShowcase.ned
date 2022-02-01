//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.hiddennode;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.unitdisk.UnitDiskRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network HiddenNodeShowcase
{
    parameters:
        @display("b=297,203;bgb=736.6,557.784");
        @figure[rcvdPkText](type=indicatorText; pos=420,20; anchor=w; font=,20; textFormat="packets received: %g"; initialValue=0);
        @statistic[packetReceived](source=hostB.udp.packetReceived; record=figure(count); targetFigure=rcvdPkText);
    submodules:

        hostA: WirelessHost {
            @display("p=300,272");
        }
        hostB: WirelessHost {
            @display("p=450,100");
        }
        hostC: WirelessHost {
            @display("p=600,272");
        }
        radioMedium: UnitDiskRadioMedium {
            @display("p=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,200");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,300");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,400");
        }
}

