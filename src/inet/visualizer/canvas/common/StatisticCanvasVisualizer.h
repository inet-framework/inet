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
        const int networkNodeId = -1; // the network node the figure is displayed above
        cFigure *figure = nullptr;
        cFigure::Point annotationSize = cFigure::Point(NaN, NaN);
        int displayedItemsVersion = -1; // the version of the item set the figure was last given

      public:
        StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, int networkNodeId, cFigure *figure, int moduleId, simsignal_t signal, const char *unit);
        virtual ~StatisticCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    // Creates the figure that displays the value(s) as configured by the `figure` parameter,
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

    virtual StatisticVisualization *createStatisticVisualization(cComponent *module, simsignal_t signal) override;
    virtual void addStatisticVisualization(StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(StatisticVisualization *statisticVisualization) override;

    // Displays the items of a visualization on its figure: their last values, preceded by
    // their number and labels whenever the set of items changed.
    virtual void refreshFigure(StatisticCanvasVisualization *statisticCanvasVisualization) const;
    // Gives the figure the number of items and their labels. The visualizer owns the label
    // to index mapping: the items are displayed in label order, so the index of an item is
    // its position among them.
    virtual void setFigureItems(cFigure *figure, const std::map<std::string, StatisticVisualization::Item>& items) const;
    // Drops the visualizations whose module has been deleted. When the network node itself
    // is gone its visualization took the figure with it, so neither may be touched again.
    virtual void removeVisualizationsOfDeletedModules();
};

} // namespace visualizer

} // namespace inet

#endif

