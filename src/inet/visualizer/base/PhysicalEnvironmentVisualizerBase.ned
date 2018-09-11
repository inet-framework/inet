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
// This is a base module for physical environment visualizer simple modules. It
// provides the physical objects through the physical environment module for
// derived modules. The actual visualization of physical objects is done in
// derived modules.
//
// @see ~PhysicalEnvironmentCanvasVisualizer, ~PhysicalEnvironmentOsgVisualizer, ~IPhysicalEnvironmentVisualizer, ~VisualizerBase
//
simple PhysicalEnvironmentVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // the physical environment module that is displayed, refers to a top level submodule by default

        bool displayObjects = default(true);

        @class(PhysicalEnvironmentVisualizerBase);
}

