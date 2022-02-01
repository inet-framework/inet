//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.physicallayer;

import inet.visualizer.base.RadioVisualizerBase;
import inet.visualizer.contract.IRadioVisualizer;

//
// This module visualizes radios on a 3D osg scene.
//
// @see ~RadioCanvasVisualizer, ~RadioVisualizer, ~RadioVisualizerBase, ~IRadioVisualizer
//
simple RadioOsgVisualizer extends RadioVisualizerBase like IRadioVisualizer
{
    parameters:
        @class(RadioOsgVisualizer);
}

