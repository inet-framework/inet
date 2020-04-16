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

#ifndef __INET_STATISTICCANVASVISUALIZER_H
#define __INET_STATISTICCANVASVISUALIZER_H

#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualization.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API StatisticCanvasVisualizer : public StatisticVisualizerBase
{
  protected:
    class StatisticCanvasVisualization : public StatisticVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        cFigure *figure = nullptr;

      public:
        StatisticCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cFigure *figure, int moduleId, simsignal_t signal, const char *unit);
    };

  protected:
    double zIndex = NaN;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override;
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override;

  public:
    virtual ~StatisticCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICCANVASVISUALIZER_H

