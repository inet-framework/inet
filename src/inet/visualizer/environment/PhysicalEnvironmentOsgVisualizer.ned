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

package inet.visualizer.environment;

import inet.visualizer.base.PhysicalEnvironmentVisualizerBase;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;

//
// This module visualizes a physical environment on a 3D osg scene. It displays
// the physical objects present in the physical environment. The visualizer
// considers all object properties that affect appearance such as position,
// orientation, shape, colors, opacity, etc.
//
// @see ~PhysicalEnvironmentOsgVisualizer, ~PhysicalEnvironmentVisualizer, ~PhysicalEnvironmentVisualizerBase, ~IPhysicalEnvironmentVisualizer
//
simple PhysicalEnvironmentOsgVisualizer extends PhysicalEnvironmentVisualizerBase like IPhysicalEnvironmentVisualizer
{
    parameters:
        @class(PhysicalEnvironmentOsgVisualizer);
        bool enableObjectOpacity = default(true); // display physical objects potentially transparently as specified in the opacitiy attribute (might cause z-fighting for transparent objects)
}

