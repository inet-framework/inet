//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for visualizer simple modules. It simply provides the
// visualization target module for derived modules.
//
simple VisualizerBase
{
    parameters:
        string visualizationTargetModule = default("^"); // module path relative to the visualizer which determines where the visualization should appear in the user interface, parent module by default
        string visualizationSubjectModule = default("^"); // module path relative to the visualizer which determines what part of the simulation model the visualizer displays in the user interface, parent module by default
        string tags = default(""); // tag added to each visualization for disambiguation in the runtime environment
        @class(VisualizerBase);
        @display("i=block/app");
}

