//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/visualizer/networknode/NetworkNodeCanvasVisualization.h"

namespace inet {

namespace visualizer {

class INET_API StatisticCanvasVisualizer : public StatisticVisualizerBase
{
  protected:
    class CanvasCacheEntry : public CacheEntry {
      public:
        NetworkNodeCanvasVisualization *visualization = nullptr;
        cGroupFigure *figure = nullptr;
        cFigure::Point size;

      public:
        CanvasCacheEntry(const char *unit, NetworkNodeCanvasVisualization *visualization, cGroupFigure *figure, cFigure::Point size);
    };

  protected:
    virtual CacheEntry *createCacheEntry(cComponent *source, simsignal_t signal) override;
    virtual void addCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override;
    virtual void removeCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override;
    virtual void refreshStatistic(CacheEntry *cacheEntry) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICCANVASVISUALIZER_H

