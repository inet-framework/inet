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

package inet.visualizer.contract;

//
// This module interface allows configuring different ~IIntegratedVisualizer
// submodules from INI files. Integrated visualizers are compound modules, they
// don't display anything on their own. Instead they combine multiple
// visualizers, most often simple modules, to provide a convenient way to add
// complex visualizations to a simulation.
//
// @see ~IntegratedCanvasVisualizer, ~IntegratedOsgVisualizer, ~IntegratedVisualizer
//
moduleinterface IIntegratedVisualizer
{
    parameters:
        @display("i=block/app");
}

