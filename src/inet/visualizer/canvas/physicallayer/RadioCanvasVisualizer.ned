//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.RadioVisualizerBase;
import inet.visualizer.contract.IRadioVisualizer;

//
// Visualizes radio modules on a 2D canvas. Displays icons and state information
// for radio equipment in network nodes.
//
// The visualization includes:
// - Radio state indicators (transmitting, receiving, idle)
// - Power indicators showing transmission power levels
// - Mode indicators showing the current radio mode
// - Automatic updates when radio states change
//
// Particularly useful for understanding wireless network behavior, debugging radio
// state transitions, and monitoring transmission activities in the simulation.
//
// @see ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase, ~IRadioVisualizer
//
simple RadioCanvasVisualizer extends RadioVisualizerBase like IRadioVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        @class(RadioCanvasVisualizer);
}
