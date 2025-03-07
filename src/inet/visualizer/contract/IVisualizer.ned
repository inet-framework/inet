//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for all visualizer modules. Visualizers add graphical annotations
// to network elements or draw lines/shapes on the canvas to represent various
// aspects of the simulation. This interface defines the common parameters
// needed by all visualizers to determine what to visualize and where to
// display the visualization.
//
moduleinterface IVisualizer
{
    parameters:
        string visualizationTargetModule; // Module path that determines where the visualization appears in the user interface
        string visualizationSubjectModule; // Module path that determines what part of the simulation model is visualized
        @display("i=block/app");
}
