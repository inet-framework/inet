//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.power;

import inet.visualizer.base.EnergyStorageVisualizerBase;
import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// Visualizes energy storage devices in a 3D scene using OpenSceneGraph (OSG).
// Displays power sources such as batteries and their current charge levels.
//
// The visualization includes:
// - 3D indicators showing energy storage charge levels TODO 3D?
// - Color-coded representations of remaining energy
// - Automatic updates as energy levels change during simulation
// - Support for various energy storage types (batteries, capacitors, etc.)
// - Filtering options to visualize only specific energy storage modules
//
// Particularly useful for understanding power consumption patterns in wireless
// networks, analyzing battery lifetime in mobile devices, debugging energy
// harvesting systems, and creating visualizations of energy-constrained
// networks in 3D view.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
simple EnergyStorageOsgVisualizer extends EnergyStorageVisualizerBase like IEnergyStorageVisualizer
{
    parameters:
        @class(EnergyStorageOsgVisualizer);
}
