//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.power;

import inet.visualizer.base.EnergyStorageVisualizerBase;
import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// This module visualizes energyStorages on a 3D osg scene.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
simple EnergyStorageOsgVisualizer extends EnergyStorageVisualizerBase like IEnergyStorageVisualizer
{
    parameters:
        @class(EnergyStorageOsgVisualizer);
}

