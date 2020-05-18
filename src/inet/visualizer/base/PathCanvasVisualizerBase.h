//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PATHCANVASVISUALIZERBASE_H
#define __INET_PATHCANVASVISUALIZERBASE_H

#include "inet/common/figures/LabeledPolylineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathCanvasVisualizerBase : public PathVisualizerBase
{
  protected:
    class INET_API PathCanvasVisualization : public PathVisualization {
      public:
        LabeledPolylineFigure *figure = nullptr;

      public:
        PathCanvasVisualization(const char *label, const std::vector<int>& path, LabeledPolylineFigure *figure);
        virtual ~PathCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *pathGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const PathVisualization *createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const override;
    virtual void addPathVisualization(const PathVisualization *pathVisualization) override;
    virtual void removePathVisualization(const PathVisualization *pathVisualization) override;
    virtual void setAlpha(const PathVisualization *pathVisualization, double alpha) const override;
    virtual void refreshPathVisualization(const PathVisualization *pathVisualization, cPacket *packet) override;

  public:
    virtual ~PathCanvasVisualizerBase();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PATHCANVASVISUALIZERBASE_H

