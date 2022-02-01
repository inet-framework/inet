//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUECANVASVISUALIZER_H
#define __INET_QUEUECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/QueueFigure.h"
#include "inet/visualizer/base/QueueVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API QueueCanvasVisualizer : public QueueVisualizerBase
{
  protected:
    class INET_API QueueCanvasVisualization : public QueueVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        QueueFigure *figure = nullptr;

      public:
        QueueCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, QueueFigure *figure, queueing::IPacketQueue *queue);
        virtual ~QueueCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual QueueVisualization *createQueueVisualization(queueing::IPacketQueue *queue) const override;
    virtual void addQueueVisualization(const QueueVisualization *queueVisualization) override;
    virtual void removeQueueVisualization(const QueueVisualization *queueVisualization) override;
    virtual void refreshQueueVisualization(const QueueVisualization *queueVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

