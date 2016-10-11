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
// This is a base module for mobility visualizer simple modules. It subscribes
// for the mobilityStateChangedSignal at the module determined by the
// subscriptionModule parameter. The actual visualization of mobility is done in
// derived modules.
//
// @see ~MobilityCanvasVisualizer, ~MobilityOsgVisualizer, ~IMobilityVisualizer, ~MobilityVisualizerBase
//
simple MobilityVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for mobility signals, root module by default
        // orientation
        bool displayOrientation = default(false); // display an arc indicating the current orientation, disabled by default
        string orientationLineColor = default("black"); // orientation line color, black by default
        double orientationLineWidth = default(1); // orientation line width
        // velocity
        bool displayVelocity = default(false); // display an arrow indicating the current velocity, disabled by default
        string velocityLineColor = default("black"); // velocity line color, black by default
        double velocityLineWidth = default(1); // velocity line width
        string velocityLineStyle = default("solid"); // velocity line style (solid, dashed, dotted)
        // movement trail
        bool displayMovementTrail = default(false); // display a line along the recent path of mobilities, disabled by default
        string movementTrailLineColor = default("auto"); // movement trail line color
        double movementTrailLineWidth = default(1); // movement trail line width
        int trailLength = default(100); // number of sections in the trail
        @class(MobilityVisualizerBase);
}
