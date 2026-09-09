//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.scene;

import inet.visualizer.base.NetworkConnectionVisualizerBase;
import inet.visualizer.contract.INetworkConnectionVisualizer;

//
// Visualizes multiple network connections on a 3D VulkanSceneGraph scene.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionVisualizer, ~NetworkConnectionVisualizerBase, ~INetworkConnectionVisualizer
//
simple NetworkConnectionVsgVisualizer extends NetworkConnectionVisualizerBase like INetworkConnectionVisualizer
{
    parameters:
        @class(NetworkConnectionVsgVisualizer);
}
