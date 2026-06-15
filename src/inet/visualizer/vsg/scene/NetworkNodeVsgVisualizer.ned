//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.scene;

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// Visualizes multiple network nodes on a 3D VulkanSceneGraph scene. The
// visualizer maintains a `NetworkNodeVsgVisualization` instance for each network
// node which can be annotated by other visualizers.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeVsgVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        @class(NetworkNodeVsgVisualizer);
        bool displayModuleName = default(true); // Display network node name, enabled by default
}
