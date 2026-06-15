//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgsmoketest;

import inet.mobility.static.StationaryMobility;
import inet.visualizer.vsg.scene.NetworkConnectionVsgVisualizer;
import inet.visualizer.vsg.scene.NetworkNodeVsgVisualizer;
import inet.visualizer.vsg.scene.SceneVsgVisualizer;

//
// A minimal "network node" for the VSG 3D smoke test: a network node with a
// StationaryMobility submodule (so the visualizers get a well-defined position via
// getPosition() at init time) and an inout gate vector so it can be wired up. The
// gates are left internally unconnected (allowunconnected) — there's no protocol
// stack; this is purely for topology visualization.
//
module VsgSmokeNode
{
    parameters:
        @networkNode;
        @display("i=device/pc");
    gates:
        inout g[];
    submodules:
        mobility: StationaryMobility {
            @display("p=60,60;is=s");
        }
    connections allowunconnected:
}

//
// Smallest network that exercises the VulkanSceneGraph 3D visualizer end-to-end:
// SceneVsgVisualizer (scene/floor/axes/viewpoint) + NetworkNodeVsgVisualizer (a 3D
// marker per node) + NetworkConnectionVsgVisualizer (a line per wired connection).
// Run under Qtenv and switch to the 3D view.
//
network VsgSmokeTest
{
    @display("bgb=300,300");
    submodules:
        sceneVisualizer: SceneVsgVisualizer {
            @display("p=20,20");
        }
        networkNodeVisualizer: NetworkNodeVsgVisualizer {
            @display("p=20,60");
        }
        networkConnectionVisualizer: NetworkConnectionVsgVisualizer {
            @display("p=20,100");
        }
        host0: VsgSmokeNode {
            @display("p=80,80");
        }
        host1: VsgSmokeNode {
            @display("p=220,80");
        }
        host2: VsgSmokeNode {
            @display("p=150,230");
        }
        router: VsgSmokeNode {
            @display("p=150,150;i=abstract/router");
        }
    connections allowunconnected:
        router.g++ <--> host0.g++;
        router.g++ <--> host1.g++;
        router.g++ <--> host2.g++;
}
