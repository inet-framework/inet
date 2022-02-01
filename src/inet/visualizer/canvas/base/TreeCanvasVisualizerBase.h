//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TREECANVASVISUALIZERBASE_H
#define __INET_TREECANVASVISUALIZERBASE_H

#include "inet/common/figures/LabeledPolylineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/TreeVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API TreeCanvasVisualizerBase : public TreeVisualizerBase
{
  protected:
    class INET_API TreeCanvasVisualization : public TreeVisualization {
      public:
        std::vector<LabeledPolylineFigure *> figures;

      public:
        TreeCanvasVisualization(const std::vector<std::vector<int>>& tree, std::vector<LabeledPolylineFigure *>& figure);
        virtual ~TreeCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *treeGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const TreeVisualization *createTreeVisualization(const std::vector<std::vector<int>>& tree) const override;
    virtual void addTreeVisualization(const TreeVisualization *treeVisualization) override;
    virtual void removeTreeVisualization(const TreeVisualization *treeVisualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

