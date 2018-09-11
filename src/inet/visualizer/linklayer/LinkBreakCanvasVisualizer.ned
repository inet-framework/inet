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

import inet.visualizer.base.LinkBreakVisualizerBase;
import inet.visualizer.contract.ILinkBreakVisualizer;

//
// This module visualizes link breaks on a 2D canvas. It displays an icon
// for each link break in the network. See the documentation of the base
// module for what constitutes a link break. The icon is positioned at the
// middle point of the link.
//
// @see ~LinkBreakOsgVisualizer, ~LinkBreakVisualizer, ~LinkBreakVisualizerBase, ~ILinkBreakVisualizer
//
simple LinkBreakCanvasVisualizer extends LinkBreakVisualizerBase like ILinkBreakVisualizer
{
    parameters:
        double zIndex = default(2); // determines the drawing order of figures relative to other visualizers
        @class(LinkBreakCanvasVisualizer);
}

