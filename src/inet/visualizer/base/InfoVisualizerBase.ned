//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for submodule information visualizer simple modules.
// It displays some information on network nodes originating from one of their
// corresponding submodules. No information is displayed by default, see module
// parameters for how to enable the visualization. The actual visualization is
// done in derived modules.
//
// The format string can contain the following directives:
//  - %n module full name
//  - %p module full path
//  - %t display string text
//  - %s cModule::str()
//
// @see ~InfoCanvasVisualizer, ~InfoOsgVisualizer, ~IInfoVisualizer, ~VisualizerBase
//
simple InfoVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayInfos = default(true);

        string modules @mutable = default(""); // specifies the path of the submodules of network nodes, no submodules by default
        string format @mutable = default("%t"); // determines what is displayed on network nodes

        string font = default("<default>"); // info text font, automatic by default
        string textColor = default("black"); // info text font color, black by default
        string backgroundColor = default("white"); // info text background color, white by default
        double opacity = default(1.0); // opacity of the visualization
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(InfoVisualizerBase);
}

