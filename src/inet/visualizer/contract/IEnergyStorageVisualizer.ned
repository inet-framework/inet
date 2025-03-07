//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface for energy storage visualizers. Energy storage visualizers display
// the charge level and state of energy storage components in network nodes.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer,
// ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase
//
moduleinterface IEnergyStorageVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
