//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.common;

import inet.visualizer.base.InfoVisualizerBase;
import inet.visualizer.contract.IInfoVisualizer;

//
// Visualizes some information of a submodule for each network node
// on a 3D VulkanSceneGraph scene.
//
// @see ~InfoCanvasVisualizer, ~InfoVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
simple InfoVsgVisualizer extends InfoVisualizerBase like IInfoVisualizer
{
    parameters:
        @class(InfoVsgVisualizer);
}
