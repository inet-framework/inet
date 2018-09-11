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
// This is a base module for visualizer simple modules. It simply provides the
// visualization target module for derived modules.
//
simple VisualizerBase
{
    parameters:
        string visualizationTargetModule = default("^"); // module path relative to the visualizer which determines where the visualization should appear in the user interface, parent module by default
        string visualizationSubjectModule = default("^"); // module path relative to the visualizer which determines what part of the simulation model the visualizer displays in the user interface, parent module by default
        string tags = default(""); // tag added to each visualization for disambiguation in the runtime environment
        @class(VisualizerBase);
        @display("i=block/app");
}

