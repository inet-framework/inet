//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRACINGOBSTACLELOSSOSGVISUALIZER_H
#define __INET_TRACINGOBSTACLELOSSOSGVISUALIZER_H

#include <osg/Group>

#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API TracingObstacleLossOsgVisualizer : public TracingObstacleLossVisualizerBase
{
  protected:
    class INET_API ObstacleLossOsgVisualization : public ObstacleLossVisualization {
      public:
        osg::Group *node = nullptr;

      public:
        ObstacleLossOsgVisualization(osg::Group *node) : node(node) {}
    };

  protected:
    osg::Group *obstacleLossNode = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const ObstacleLossVisualization *createObstacleLossVisualization(const physicallayer::ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const override;
    virtual void addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization) override;
    virtual void removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization) override;
    virtual void setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

