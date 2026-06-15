//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.transportlayer;

import inet.visualizer.base.TransportConnectionVisualizerBase;
import inet.visualizer.contract.ITransportConnectionVisualizer;

//
// Visualizes transport layer connections in a 3D VulkanSceneGraph scene.
// It displays a pair of colored icons for each connection at the network nodes
// where the connection starts or ends.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionVisualizer, ~TransportConnectionVisualizerBase, ~ITransportConnectionVisualizer
//
simple TransportConnectionVsgVisualizer extends TransportConnectionVisualizerBase like ITransportConnectionVisualizer
{
    parameters:
        @class(TransportConnectionVsgVisualizer);
}
