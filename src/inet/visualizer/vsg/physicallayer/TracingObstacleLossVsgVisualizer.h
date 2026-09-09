//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRACINGOBSTACLELOSSVSGVISUALIZER_H
#define __INET_TRACINGOBSTACLELOSSVSGVISUALIZER_H

#include <vsg/nodes/Group.h>

#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API TracingObstacleLossVsgVisualizer : public TracingObstacleLossVisualizerBase
{
  protected:
    class INET_API ObstacleLossVsgVisualization : public ObstacleLossVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Group> node;

      public:
        ObstacleLossVsgVisualization(::vsg::ref_ptr<::vsg::Group> node) : node(node) {}
    };

  protected:
    // Container group attached to the scene; individual events are children of this group.
    ::vsg::ref_ptr<::vsg::Group> obstacleLossNode;

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
