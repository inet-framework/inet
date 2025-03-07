//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for submodule information visualizer simple modules.
// It displays textual information about network nodes and their submodules as
// annotations attached to the nodes.
//
// The visualizer allows displaying various properties of modules, such as their
// names, paths, or display string text. This is particularly useful for debugging,
// creating informative visualizations, or annotating specific aspects of the
// simulation model.
//
// By default, information display is enabled, but no modules are selected.
// To visualize module information, set the modules parameter to specify which
// submodules to consider, and use the format parameter to determine what
// information is displayed.
//
// The actual rendering of information annotations is implemented in derived
// modules for specific user interfaces (canvas or OSG).
//
// The format string can contain the following directives:
//  - %n module full name
//  - %p module full path
//  - %t display string text
//  - %s `cModule`::str()
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~IInfoVisualizer, ~VisualizerBase
//
simple InfoVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayInfos = default(true);

        string modules @mutable = default(""); // Specifies the path of the submodules of network nodes, no submodules by default
        string format @mutable = default("%t"); // Determines what is displayed on network nodes

        string font = default("<default>"); // Info text font, automatic by default
        string textColor = default("black"); // Info text font color
        string textAlignment @enum("left", "right", "center") = default("left"); // Alignment of multi-line text
        string backgroundColor = default("white"); // Text box fill color, specify an empty string for no fill
        string lineColor = default("black"); // Text box outline color, specify an empty string for no line
        double opacity = default(1.0); // Opacity of the visualization
        string placementHint = default("top"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(InfoVisualizerBase);
}
