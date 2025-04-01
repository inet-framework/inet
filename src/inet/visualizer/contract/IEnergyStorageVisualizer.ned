//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IEnergyStorageVisualizer
// submodules from INI files. Such a visualizer displays energy storages.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer,
// ~EnergyStorageVisualizer, ~EnergyStorageVisualizerBase
//
moduleinterface IEnergyStorageVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

