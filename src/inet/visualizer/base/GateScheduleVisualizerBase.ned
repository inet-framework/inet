//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Base module for gate visualizer simple modules.
//
// Format string directives:
//	- %i: interface name
// 	- %m: gate module name
// 	- %d: gate module display name
//  - %D: gate module display name if exists, otherwise gate module name
//
// @see ~GateScheduleCanvasVisualizer, ~GateScheduleOsgVisualizer, ~IGateScheduleVisualizer, ~VisualizerBase
//
simple GateScheduleVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayGateSchedules = default(false);
        string labelFormat = default("%i %D");

        string gateFilter = default("**"); // Which gates are considered, matches all gates by default

        double width = default(100);
        double height = default(8);
        double spacing = default(2);
        string placementHint = default("right"); // Annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        double displayDuration @mutable @unit(s) = default(0s); // Gate operation time duration visualized in the amount of space available, use gate period by default
        double currentTimePosition = default(width / 2); // Position of the current simulation time in the visualization, horizontally centered by default

        @class(GateScheduleVisualizerBase);
}

