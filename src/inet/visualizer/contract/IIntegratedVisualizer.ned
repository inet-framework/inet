//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for integrated visualizers. Integrated visualizers are compound
// modules that contain all visualizer module types to provide a comprehensive
// visualization of the network simulation.
//
// Integrated visualizers do not display anything on their own but rather
// coordinate multiple specialized visualizers to create a complete visual
// representation of the simulation, offering a convenient way to add
// complex visualizations with a single module. TODO they don't coordinate they just contain the various visualizers
// something like: 
// Integrated visualizers do not display anything on their own; visualization is provided by its specialized
// visualizer submodules. This offers a convenient way to add all visualizer types to simulations.
//
// @see ~IntegratedCanvasVisualizer, ~IntegratedOsgVisualizer, ~IntegratedVisualizer
//
moduleinterface IIntegratedVisualizer
{
    parameters:
        @display("i=block/app");
}
