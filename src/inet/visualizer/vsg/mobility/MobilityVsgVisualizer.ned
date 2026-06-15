//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.mobility;

import inet.visualizer.base.MobilityVisualizerBase;
import inet.visualizer.contract.IMobilityVisualizer;

//
// Visualizes multiple mobilities on a 3D VulkanSceneGraph scene. It displays
// recent movement of each mobile node as a polyline trail.
//
// @see ~MobilityCanvasVisualizer, ~MobilityVisualizer, ~MobilityVisualizerBase, ~IMobilityVisualizer
//
simple MobilityVsgVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    parameters:
        @class(MobilityVsgVisualizer);
}
