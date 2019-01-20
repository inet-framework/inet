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

#ifndef __INET_MOBILITYVISUALIZERBASE_H
#define __INET_MOBILITYVISUALIZERBASE_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

class INET_API MobilityVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API MobilityVisualization {
      public:
        IMobility *mobility = nullptr;

      public:
        MobilityVisualization(IMobility *mobility);
    };
  protected:
    /** @name Parameters */
    //@{
    bool displayMobility = false;
    double animationSpeed = NaN;
    ModuleFilter moduleFilter;
    // position
    bool displayPositions = false;
    double positionCircleRadius = NaN;
    double positionCircleLineWidth = NaN;
    ColorSet positionCircleLineColorSet;
    ColorSet positionCircleFillColorSet;
    // orientation
    bool displayOrientations = false;
    double orientationPieRadius = NaN;
    double orientationPieSize = NaN;
    double orientationPieOpacity = NaN;
    cFigure::Color orientationLineColor;
    cFigure::LineStyle orientationLineStyle;
    double orientationLineWidth = NaN;
    cFigure::Color orientationFillColor;
    // velocity
    bool displayVelocities = false;
    double velocityArrowScale = NaN;
    cFigure::Color velocityLineColor;
    cFigure::LineStyle velocityLineStyle;
    double velocityLineWidth = NaN;
    // movement trail
    bool displayMovementTrails = false;
    bool autoMovementTrailLineColor = false;
    ColorSet movementTrailLineColorSet;
    cFigure::LineStyle movementTrailLineStyle;
    double movementTrailLineWidth = NaN;
    int trailLength = -1;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

  public:
    virtual ~MobilityVisualizerBase();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MOBILITYVISUALIZERBASE_H

