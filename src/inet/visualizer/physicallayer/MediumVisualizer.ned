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

import inet.visualizer.contract.IMediumVisualizer;

//
// This module integrates canvas and osg medium visualizers into a single module.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~MediumVisualizerBase, ~IMediumVisualizer
//
module MediumVisualizer like IMediumVisualizer
{
    parameters:
        @display("i=block/app");
        *.visualizationTargetModule = default("^.^");
        *.visualizationSubjectModule = default("^.^");
    submodules:
        canvasVisualizer: <default("MediumCanvasVisualizer")> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=100,100");
        }
        osgVisualizer: <default("MediumOsgVisualizer")> like IMediumVisualizer if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

