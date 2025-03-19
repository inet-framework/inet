//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IRadioVisualizer
// submodules from INI files. Such a visualizer displays radios.
//
// @see ~RadioCanvasVisualizer, ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase
//
moduleinterface IRadioVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}

