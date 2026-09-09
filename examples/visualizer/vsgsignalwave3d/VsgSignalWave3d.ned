//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgsignalwave3d;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// Showcase for the VOLUMETRIC shader signal wavefront in true 3D
// (mediumVisualizer.signalShape = "sphere" + signalWaveShader = true). A swarm of
// four drones flies through the air (3D LinearMobility with a climb/descent
// elevation) around a ground control station, all exchanging UDP traffic. Each
// transmission expands as a translucent, rippling GLOWING SPHERE centred at the
// emitter's 3D position -- the ripple opacity is computed per pixel by a fragment
// shader across concentric shells, so the waves flow outward while the node is
// sending and then fade. Several colour-coded spheres pulse in the air at once.
// Run under Qtenv, switch to the 3D view, orbit the camera, and press Run.
//
network VsgSignalWave3dShowcase
{
    @display("bgb=300,300");
    submodules:
        visualizer: IntegratedVsgVisualizer {
            @display("p=30,30");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=30,90");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=30,150");
        }
        gcs: WirelessHost { // ground control station
            @display("p=150,150;i=device/antennatower");
        }
        drone1: WirelessHost {
            @display("p=80,80;i=misc/drone");
        }
        drone2: WirelessHost {
            @display("p=220,80;i=misc/drone");
        }
        drone3: WirelessHost {
            @display("p=220,220;i=misc/drone");
        }
        drone4: WirelessHost {
            @display("p=80,220;i=misc/drone");
        }
}
