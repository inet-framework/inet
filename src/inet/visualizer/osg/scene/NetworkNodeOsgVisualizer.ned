//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// This module visualizes multiple network nodes on a 3D osg scene. The visual
// representation is either a 3D external model or a 2D icon that is rotated
// automatically towards the camera. The 3D external model is specified in the
// osgModel module parameter of the network node. The 2D icon is the image
// specified in the display string of the network node. The visualizer maintains
// a NetworkNodeOsgVisualization instance for each network node which can be
// annotated by other visualizers.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeOsgVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        @class(NetworkNodeOsgVisualizer);
        bool displayModuleName = default(true); // display network node name, enabled by default
}

