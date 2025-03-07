//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for gate schedule visualizer simple modules. It visualizes
// the state and timing of gates in Time-Sensitive Networking (TSN) applications,
// showing when gates are open or closed over time.
//
// By default, gate schedules are not displayed. To enable visualization, set the
// displayGateSchedules parameter to true and use the gateFilter to select which
// gates to visualize.
//
// The visualizer shows a timeline representation of gate operations, with the
// current simulation time indicated. This helps understand the temporal behavior
// of time-aware networks, gate control lists (GCLs), and scheduled traffic.
//
// The actual rendering of gate schedules is implemented in derived modules for
// specific user interfaces (canvas or OSG).
//
// Format string directives:
//  - %i: interface name
//  - %m: gate module name
//  - %d: gate module display name
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
