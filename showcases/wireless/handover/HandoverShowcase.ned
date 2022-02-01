//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.handover;

import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network HandoverShowcase
{
    parameters:
        @display("bgb=640,420");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100");
        }
        host: WirelessHost {
            parameters:
                @display("p=50,280;r=,,#707070");
        }
        ap1: AccessPoint {
            parameters:
                @display("p=100,350;r=,,#707070");
        }
        ap2: AccessPoint {
            parameters:
                @display("p=500,350;r=,,#707070");
        }
}

