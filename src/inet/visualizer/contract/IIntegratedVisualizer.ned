//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for integrated visualizers. Integrated visualizers are compound
// modules that combine multiple visualizer modules to provide a comprehensive
// visualization of the network simulation.
//
// Integrated visualizers do not display anything on their own but rather
// coordinate multiple specialized visualizers to create a complete visual
// representation of the simulation, offering a convenient way to add
// complex visualizations with a single module.
//
// @see ~IntegratedCanvasVisualizer, ~IntegratedOsgVisualizer, ~IntegratedVisualizer
//
moduleinterface IIntegratedVisualizer
{
    parameters:
        @display("i=block/app");
}
