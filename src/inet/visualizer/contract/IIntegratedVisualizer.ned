//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IIntegratedVisualizer
// submodules from INI files. Integrated visualizers are compound modules, they
// don't display anything on their own. Instead they combine multiple
// visualizers, most often simple modules, to provide a convenient way to add
// complex visualizations to a simulation.
//
// @see ~IntegratedCanvasVisualizer, ~IntegratedOsgVisualizer, ~IntegratedVisualizer
//
moduleinterface IIntegratedVisualizer
{
    parameters:
        @display("i=block/app");
}

