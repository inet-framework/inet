//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEOSGVISUALIZER_H
#define __INET_QUEUEOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/QueueVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API QueueOsgVisualizer : public QueueVisualizerBase
{
  protected:
    class INET_API QueueOsgVisualization : public QueueVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        QueueOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, queueing::IPacketQueue *queue);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override;
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

