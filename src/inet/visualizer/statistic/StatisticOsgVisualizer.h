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

#ifndef __INET_STATISTICOSGVISUALIZER_H
#define __INET_STATISTICOSGVISUALIZER_H

#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/networknode/NetworkNodeOsgVisualization.h"

#ifdef WITH_OSG
#include <osg/Node>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

class INET_API StatisticOsgVisualizer : public StatisticVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class OsgCacheEntry : public CacheEntry {
      public:
        NetworkNodeOsgVisualization *visualization = nullptr;
        osg::Node *node = nullptr;

      public:
        OsgCacheEntry(const char *unit, NetworkNodeOsgVisualization *visualization, osg::Node *node);
    };

  protected:
    virtual CacheEntry *createCacheEntry(cComponent *source, simsignal_t signal) override;
    virtual void addCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override;
    virtual void removeCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override;
    virtual void refreshStatistic(CacheEntry *cacheEntry) override;

#else // ifdef WITH_OSG

  protected:
    virtual CacheEntry *createCacheEntry(cComponent *source, simsignal_t signal) override { return nullptr;}
    virtual void addCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override {}
    virtual void removeCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry) override {}
    virtual void refreshStatistic(CacheEntry *cacheEntry) override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICOSGVISUALIZER_H

