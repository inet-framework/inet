//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.physicallayer;

import inet.visualizer.base.RadioVisualizerBase;
import inet.visualizer.contract.IRadioVisualizer;

//
// This module visualizes radios on a 2D canvas.
//
// @see ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase, ~IRadioVisualizer
//
simple RadioCanvasVisualizer extends RadioVisualizerBase like IRadioVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        @class(RadioCanvasVisualizer);
}

