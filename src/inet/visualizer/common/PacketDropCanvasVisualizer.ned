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

package inet.visualizer.common;

import inet.visualizer.base.PacketDropVisualizerBase;
import inet.visualizer.contract.IPacketDropVisualizer;

//
// This module visualizes packet drops on a 2D canvas. It displays an icon for
// each packet drop in the network. See the documentation of the base module for
// what constitutes a packet drop.
//
// @see ~PacketDropOsgVisualizer, ~PacketDropVisualizer, ~PacketDropVisualizerBase, ~IPacketDropVisualizer
//
simple PacketDropCanvasVisualizer extends PacketDropVisualizerBase like IPacketDropVisualizer
{
    parameters:
        double zIndex = default(10); // determines the drawing order of figures relative to other visualizers
        double dx = default(32); // specifies how far the animation turns over along the X axis
        double dy = default(64); // specifies how far the animation turns over along the Y axis 
        @class(PacketDropCanvasVisualizer);
}

