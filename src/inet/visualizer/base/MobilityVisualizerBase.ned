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
// for the mobilityStateChanged signal at the module determined by the
// visualizationSubjectModule parameter. The actual visualization of mobility is done in
// derived modules.
//
// @see ~MobilityCanvasVisualizer, ~MobilityOsgVisualizer, ~IMobilityVisualizer, ~VisualizerBase
//
simple MobilityVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer module, refers to a sibling submodule by default

        bool displayMobility = default(true); // display mobility, enabled by default
        double animationSpeed = default(0); // animation speed while objects are moving around, value must be in the range (0, +inf)

        string moduleFilter @mutable = default("**.mobility"); // which mobilities are considered, matches all mobilities by default

        bool displayPositions = default(false); // display a circle indicating the current position, disabled by default
        double positionCircleRadius = default(4); // radius of the circle
        double positionCircleLineWidth = default(1); // line width of the circle
        string positionCircleLineColor @enum("light", "dark") = default("dark"); // position circle line color is a list of colors, a set of dark colors by default
        string positionCircleFillColor @enum("light", "dark") = default("dark"); // position circle fill color is a list of colors, a set of dark colors by default

        bool displayOrientations = default(false); // display a pie indicating the current orientation, disabled by default
        double orientationPieRadius = default(32); // radius of the pie
        double orientationPieSize = default(0.2); // relative size of the pie compared to the full circle 
        double orientationPieOpacity = default(0.25); // opacity of the pie
        string orientationLineColor = default("black"); // orientation line color, black by default
        string orientationLineStyle = default("solid"); // orientation line style (solid, dashed, dotted)
        double orientationLineWidth = default(1); // orientation line width
        string orientationFillColor = default("blue"); // orientation fill color, blue by default

        bool displayVelocities = default(false); // display an arrow indicating the current velocity, disabled by default
        double velocityArrowScale = default(1); // velocity scaling factor to pixels
        string velocityLineColor = default("black"); // velocity line color, black by default
        string velocityLineStyle = default("solid"); // velocity line style (solid, dashed, dotted)
        double velocityLineWidth = default(1); // velocity line width

        bool displayMovementTrails = default(false); // display a line along the recent path of mobilities, disabled by default
        string movementTrailLineColor @enum("light", "dark") = default("dark"); // movement trail line color is a list of colors, a set of dark colors by default
        string movementTrailLineStyle = default("solid"); // movement trail line style (solid, dashed, dotted)
        double movementTrailLineWidth = default(1); // movement trail line width
        int trailLength = default(100); // number of sections in the trail

        @class(MobilityVisualizerBase);
}

