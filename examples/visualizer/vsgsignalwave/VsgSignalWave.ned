//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgsignalwave;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// Showcase for the GLSL shader-based signal wavefront in the VSG visualizer
// (mediumVisualizer.signalWaveShader = true). The ripple opacity is computed per
// pixel by a fragment shader whose waveOffset advances every frame, so the waves
// flow continuously outward like the OSG signal visualization; the disc stays lit
// for the whole transmission (signalTransmissionAnimationSpeed), and when a
// transmission finishes the ring expands, empties from the centre, and fades.
//
// Four hosts sit around a central hub (~110m spokes, within communication range),
// all exchanging UDP traffic, so several translucent colour-coded ripple discs
// overlap and pulse across the scene. host2 drifts to add some motion. Run under
// Qtenv, switch to the 3D view, and press Run.
//
network VsgSignalWaveShowcase
{
    @display("bgb=400,400");
    submodules:
        visualizer: IntegratedVsgVisualizer {
            @display("p=40,40");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=40,110");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=40,180");
        }
        hub: WirelessHost {
            @display("p=200,200");
        }
        host1: WirelessHost {
            @display("p=200,90");
        }
        host2: WirelessHost {
            @display("p=310,200");
        }
        host3: WirelessHost {
            @display("p=200,310");
        }
        host4: WirelessHost {
            @display("p=90,200");
        }
}
