//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.linklayer;

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// Visualizes link breaks in a 3D VulkanSceneGraph (VSG) scene. Displays a
// colored sphere at the midpoint of each broken link; the sphere fades out over
// time according to the fadeOutMode/fadeOutTime parameters.
//
// The OSG twin (~LinkBreakOsgVisualizer) renders a textured-quad billboard using
// the configured icon image. This VSG implementation approximates that with a
// colored sphere (iconTintColor) because VsgUtils does not yet expose a
// textured-billboard helper.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakVsgVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        @class(LinkBreakVsgVisualizer);
}
