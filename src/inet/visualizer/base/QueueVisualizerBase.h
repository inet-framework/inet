//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEVISUALIZERBASE_H
#define __INET_QUEUEVISUALIZERBASE_H

#include "inet/common/INETDefs.h"
#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/QueueFilter.h"

namespace inet {

namespace visualizer {

class INET_API QueueVisualizerBase : public VisualizerBase
{
  protected:
    class INET_API QueueVisitor : public cVisitor {
      public:
        std::vector<queueing::IPacketQueue *> queues;

      public:
        virtual bool visit(cObject *object) override;
    };

    class INET_API QueueVisualization {
      public:
        queueing::IPacketQueue *queue = nullptr;

      public:
        QueueVisualization(queueing::IPacketQueue *queue);
        virtual ~QueueVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayQueues = false;
    QueueFilter queueFilter;
    cFigure::Color color;
    double width;
    double height;
    double spacing;
    Placement placementHint;
    double placementPriority;
    //@}

    std::vector<const QueueVisualization *> queueVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const = 0;
    virtual void addQueueVisualization(const QueueVisualization *queueVisualization);
    virtual void addQueueVisualizations();
    virtual void removeQueueVisualization(const QueueVisualization *queueVisualization);
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const = 0;
    virtual void removeAllQueueVisualizations();
};

} // namespace visualizer

} // namespace inet

#endif

