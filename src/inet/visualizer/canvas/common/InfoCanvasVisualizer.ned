//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.common;

import inet.visualizer.base.InfoVisualizerBase;
import inet.visualizer.contract.IInfoVisualizer;

//
// Visualizes module information on a 2D canvas. Displays customizable text
// annotations for network nodes based on their submodules.
//
// The visualization includes:
// - Text labels showing information about specific submodules
// - Customizable format strings with module names, paths, and display strings
// - Configurable appearance (font, colors, alignment, background)
// - Flexible placement options around network nodes
//
// Particularly useful for displaying module status information, debugging
// module configurations, creating informative labels for presentations,
// and providing context for complex simulations.
//
// @see ~InfoOsgVisualizer, ~InfoVisualizer, ~InfoVisualizerBase, ~IInfoVisualizer
//
simple InfoCanvasVisualizer extends InfoVisualizerBase like IInfoVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        @class(InfoCanvasVisualizer);
}
