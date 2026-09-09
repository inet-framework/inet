//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEVSGVISUALIZER_H
#define __INET_QUEUEVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/QueueVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API QueueVsgVisualizer : public QueueVisualizerBase
{
  protected:
    class INET_API QueueVsgVisualization : public QueueVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; holds the current queue visual
        mutable int lastNumPackets = -1;    // cached packet count; -1 = not yet rendered
        mutable int lastMaxNumPackets = -1; // cached capacity; -1 = not yet rendered

      public:
        QueueVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, queueing::IPacketQueue *queue);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override;
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif
