//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.power;

import inet.visualizer.base.EnergyStorageVisualizerBase;
import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// Visualizes energy storages on a 3D VulkanSceneGraph scene.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
simple EnergyStorageVsgVisualizer extends EnergyStorageVisualizerBase like IEnergyStorageVisualizer
{
    parameters:
        @class(EnergyStorageVsgVisualizer);
}
