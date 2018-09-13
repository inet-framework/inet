//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.visualizer.linklayer;

import inet.visualizer.contract.IIeee80211Visualizer;

//
// This module integrates canvas and osg IEEE 802.11 association visualizers into a single module.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~Ieee80211VisualizerBase, ~IIeee80211Visualizer
//
module Ieee80211Visualizer like IIeee80211Visualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default("Ieee80211CanvasVisualizer")> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default("Ieee80211OsgVisualizer")> like IIeee80211Visualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

