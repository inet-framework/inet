//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for energy storage visualizer simple modules.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer, ~IEnergyStorageVisualizer, ~VisualizerBase
//
simple EnergyStorageVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayEnergyStorages = default(false);

        string energyStorageFilter = default("**"); // which energy storages are considered, matches all energy storages by default

        string color = default("green");
        double width = default(8);
        double height = default(24);
        double spacing = default(2);
        string placementHint = default("right"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(EnergyStorageVisualizerBase);
}

