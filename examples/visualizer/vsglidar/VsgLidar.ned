//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsglidar;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A drone swarm flying over a real LIDAR landscape. The VSG scene ground is replaced
// by an actual LIDAR point-cloud scan (mediumVisualizer... sceneVisualizer.sceneModel
// = a PLY file), coloured by elevation, recentred and auto-fit into the scene bounds.
// Five drones fly through the airspace above the terrain, exchanging UDP traffic; each
// transmission is a translucent, rippling glowing sphere expanding over the landscape.
// The (flat, meaningless-in-3D) range indicators are turned off to keep the view clean,
// leaving just the terrain + the signal wavefronts. Run under Qtenv, switch to the 3D
// view, orbit/tilt the camera, and press Run.
//
// The sceneModel mechanism works with ANY PLY point cloud; terrain.ply here is a
// subsampled USGS Sandy LIDAR tile of New York (buildings included).
//
network VsgLidarShowcase
{
    @display("bgb=800,450");
    submodules:
        visualizer: IntegratedVsgVisualizer {
            @display("p=60,60");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,140");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=60,220");
        }
        drone1: WirelessHost { // swarm leader
            @display("p=400,225;i=misc/drone");
        }
        drone2: WirelessHost {
            @display("p=150,120;i=misc/drone");
        }
        drone3: WirelessHost {
            @display("p=650,120;i=misc/drone");
        }
        drone4: WirelessHost {
            @display("p=650,330;i=misc/drone");
        }
        drone5: WirelessHost {
            @display("p=150,330;i=misc/drone");
        }
}
