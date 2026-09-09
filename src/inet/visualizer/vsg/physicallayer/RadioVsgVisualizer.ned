//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.physicallayer;

import inet.visualizer.base.RadioVisualizerBase;
import inet.visualizer.contract.IRadioVisualizer;

//
// Visualizes radios on a 3D VulkanSceneGraph scene. Adds a small sphere
// marker at the network node's position for each radio, tagged with the
// module object for picking.
//
// @see ~RadioOsgVisualizer, ~RadioVisualizer, ~RadioVisualizerBase, ~IRadioVisualizer
//
simple RadioVsgVisualizer extends RadioVisualizerBase like IRadioVisualizer
{
    parameters:
        @class(RadioVsgVisualizer);
}
