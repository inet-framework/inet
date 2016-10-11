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

package inet.visualizer.physicallayer;

import inet.visualizer.base.LinkVisualizerBase;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

//
// This module visualizes network links on a 3D osg scene. It displays a
// line for each active link in the network. See the documentation of the base
// module for what constitutes an active link. Each line fades out over time
// until the link becomes inactive unless it is reinforced by another packet.
// The line automatically follows the movement of mobile nodes.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkVisualizer, ~LinkVisualizerBase, ~IPhysicalLinkVisualizer
//
simple PhysicalLinkOsgVisualizer extends LinkVisualizerBase like IPhysicalLinkVisualizer
{
    parameters:
        lineColor = default("black"); // link line color, black by default
        @class(PhysicalLinkOsgVisualizer);
}
