//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IIeee80211Visualizer
// submodules from INI files. Such a visualizer displays IEEE 802.11 network
// interface associations.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~Ieee80211Visualizer, ~Ieee80211VisualizerBase
//
moduleinterface IIeee80211Visualizer
{
    parameters:
        @display("i=block/app");
}
