//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// This module interface allows configuring different ~IRadioVisualizer
// submodules from INI files. Such a visualizer displays radios.
//
// @see ~RadioCanvasVisualizer, ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase
//
moduleinterface IRadioVisualizer
{
    parameters:
        @display("i=block/app");
}

