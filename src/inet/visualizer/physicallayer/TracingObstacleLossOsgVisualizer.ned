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

import inet.visualizer.base.TracingObstacleLossVisualizerBase;
import inet.visualizer.contract.ITracingObstacleLossVisualizer;

//
// This module visualizes obstacle loss on a 3D osg scene. It displays a line at
// the intersections of signal propagation paths with physical objects, and
// normal vectors of physical object faces at intersection points.
//
// @see ~TracingObstacleLossOsgVisualizer, ~TracingObstacleLossVisualizer, ~TracingObstacleLossVisualizerBase, ~ITracingObstacleLossVisualizer
//
simple TracingObstacleLossOsgVisualizer extends TracingObstacleLossVisualizerBase like ITracingObstacleLossVisualizer
{
    parameters:
        @class(TracingObstacleLossOsgVisualizer);
}

