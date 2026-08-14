//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICCANVASVISUALIZER_H
#define __INET_STATISTICCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/IIndicatorFigure.h"
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
        cFigure::Point annotationSize = cFigure::Point(NaN, NaN);

      public:
        StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit);
        virtual ~StatisticCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    // Creates the figure that displays the value as configured by the `figure` parameter,
    // or by the figure template property named by the `propertyName` parameter; returns
    // nullptr if neither is given.
    virtual cFigure *createIndicatorFigure();
    // Returns the figure template property named by the `propertyName` and `propertyIndex`
    // parameters, looked up along the module path, or nullptr if `propertyName` is empty.
    virtual cProperty *findFigureTemplateProperty();
    // Creates the figure of the type given in the property (see Register_Figure()) and
    // configures it from the other attributes of the property.
    virtual cFigure *createFigure(cProperty *property) const;
    // Updates the size the network node visualizer reserves for a figure, if it changed.
    virtual void setAnnotationSize(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, const cFigure::Point& size, cFigure::Point& lastSize) const;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override;
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

