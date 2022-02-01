//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        virtual ~MobilityVisualization() {}
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

    std::map<int, MobilityVisualization *> mobilityVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual MobilityVisualization *createMobilityVisualization(IMobility *mobility) = 0;
    virtual MobilityVisualization *getMobilityVisualization(const IMobility *mobility) const;
    virtual void addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization);
    virtual void removeMobilityVisualization(const MobilityVisualization *visualization);
    virtual void removeAllMobilityVisualizations();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

