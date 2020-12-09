//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

