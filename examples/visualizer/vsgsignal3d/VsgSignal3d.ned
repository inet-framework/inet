//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgsignal3d;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A wireless network for showing VSG signal propagation as translucent expanding
// spheres in real 3D space (signalShape = "sphere"). Two ground stations sit on the
// floor within range of each other, and host3 is a "drone" that flies through the
// volume above them (3D LinearMobility with a non-zero movement elevation). As each
// node transmits, a translucent sphere expands outward from its current 3D position,
// so the drone's transmissions are visibly centred up in the air rather than on the
// ground. Because the nodes stay within range, the data-link visualizer also draws
// connection arrows. Run under Qtenv, switch to the 3D view, orbit the camera, and
// press Run.
//
network VsgSignal3dShowcase
{
    @display("bgb=300,300");
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
        host1: WirelessHost { // ground station
            @display("p=110,150;i=device/antennatower");
        }
        host2: WirelessHost { // ground station
            @display("p=190,150;i=device/antennatower");
        }
        host3: WirelessHost { // drone flying through the 3D volume
            @display("p=150,150;i=misc/drone");
        }
}
