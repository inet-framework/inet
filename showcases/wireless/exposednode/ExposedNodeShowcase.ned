//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.exposednode;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.common.medium.UnitDiskRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network ExposedNodeShowcase
{
    parameters:
        @display("bgb=1000,620");
        @figure[rcvdPkTextB](type=indicatorText; pos=40,120; anchor=w; font=,18; textFormat="packets received at hostB: %g"; initialValue=0);
        @figure[rcvdPkTextD](type=indicatorText; pos=640,120; anchor=w; font=,18; textFormat="packets received at hostD: %g"; initialValue=0);
        @statistic[packetReceivedB](source=hostB.udp.packetReceived; record=figure(count); targetFigure=rcvdPkTextB);
        @statistic[packetReceivedD](source=hostD.udp.packetReceived; record=figure(count); targetFigure=rcvdPkTextD);
    submodules:
        hostB: WirelessHost {
            @display("p=100,310");
        }
        hostA: WirelessHost {
            @display("p=350,310;r=300");
        }
        hostC: WirelessHost {
            @display("p=600,310;r=300,,red");
        }
        hostD: WirelessHost {
            @display("p=850,310");
        }
        radioMedium: UnitDiskRadioMedium {
            @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=250,50");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=400,50");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=550,50");
        }
}
