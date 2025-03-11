//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.RadioVisualizerBase;
import inet.visualizer.contract.IRadioVisualizer;

//
// Visualizes radio modules in a 3D scene using OpenSceneGraph (OSG). Displays
// icons and state information for radio modules in network nodes.
//
// The visualization includes:
// - 3D indicators for radio state (transmitting, receiving, idle)
// - Power indicators showing transmission power levels
// - Mode indicators showing the current radio mode
// - Automatic updates when radio states change
//
// Particularly useful for understanding wireless network behavior in 3D space,
// debugging radio state transitions, monitoring transmission activities.
//
// @see ~RadioCanvasVisualizer, ~RadioVisualizer, ~RadioVisualizerBase, ~IRadioVisualizer
//
simple RadioOsgVisualizer extends RadioVisualizerBase like IRadioVisualizer
{
    parameters:
        @class(RadioOsgVisualizer);
}
