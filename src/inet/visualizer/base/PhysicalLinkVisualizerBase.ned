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

package inet.visualizer.base;

//
// This is a base module for physical layer link visualizer simple modules. It
// keeps track of active links in a network. A link between two network node is
// considered active if a packet is sent recently between their physical layers.
// Active links are not displayed by default, see the module parameters for how
// to enable the visualization. The actual visualization of active links is done
// in derived modules.
//
// This module subscribes for packetReceivedFromUpper and packetSentToUpper on
// the module determined by the visualizationSubjectModule parameter. If the visualizer
// receives a packetReceivedFromUpper from a physical layer module, then it
// stores the corresponding packet id. If later on the visualizer receives a
// packetSentToUpper from another physical layer module with the same packet
// id, then it adds an active link between the source and the destination nodes.
// After a certain amount of time the link becomes inactive unless it is
// reinforced by another packet.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkOsgVisualizer, ~IPhysicalLinkVisualizer, ~VisualizerBase
//
simple PhysicalLinkVisualizerBase extends LinkVisualizerBase
{
    parameters:
        lineColor = default("olive");
        lineStyle = default("dotted");
        labelColor = default("olive");
        @class(PhysicalLinkVisualizerBase);
}

