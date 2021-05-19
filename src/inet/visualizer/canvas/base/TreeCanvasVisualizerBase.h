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

