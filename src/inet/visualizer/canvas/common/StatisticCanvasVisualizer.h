//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICCANVASVISUALIZER_H
#define __INET_STATISTICCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualization.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API StatisticCanvasVisualizer : public StatisticVisualizerBase
{
  protected:
    class INET_API StatisticCanvasVisualization : public StatisticVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        cFigure *figure = nullptr;

      public:
        StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit);
        virtual ~StatisticCanvasVisualization();
    };

    class INET_API BarSetCanvasVisualization : public BarSetVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        cGroupFigure *figure = nullptr;
        cFigure::Rectangle bounds;

      public:
        BarSetCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cGroupFigure *figure, int networkNodeId, int moduleId);
        virtual ~BarSetCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override;
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override;

    virtual BarSetVisualization *createBarSetVisualization(cComponent *source) override;
    virtual void addBarSetVisualization(BarSetVisualization *barSetVisualization) override;
    virtual void removeBarSetVisualization(BarSetVisualization *barSetVisualization) override;
    virtual void refreshChart(BarSetCanvasVisualization *barSetVisualization) const;
};

} // namespace visualizer

} // namespace inet

#endif

