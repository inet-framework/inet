//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.power;

import inet.visualizer.base.EnergyStorageVisualizerBase;
import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// This module visualizes energyStorages on a 2D canvas.
//
// @see ~EnergyStorageOsgVisualizer, ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
simple EnergyStorageCanvasVisualizer extends EnergyStorageVisualizerBase like IEnergyStorageVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(EnergyStorageCanvasVisualizer);
}

