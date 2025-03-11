//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.common;

import inet.visualizer.base.InfoVisualizerBase;
import inet.visualizer.contract.IInfoVisualizer;

//
// Visualizes module information in a 3D scene using OpenSceneGraph (OSG).
// Displays customizable text annotations for network nodes based on their submodules.
//
// The visualization includes:
// - 3D text labels showing information about specific submodules
// - Customizable format strings with module names, paths, and display strings
// - Configurable appearance (font, colors, alignment, background)
// - Flexible placement options around network nodes
//
// Particularly useful for displaying module status information in 3D space,
// debugging module configurations, creating informative labels for presentations,
// and providing context for complex simulations in a 3D view.
//
// @see ~InfoCanvasVisualizer, ~InfoVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
simple InfoOsgVisualizer extends InfoVisualizerBase like IInfoVisualizer
{
    parameters:
        @class(InfoOsgVisualizer);
}
