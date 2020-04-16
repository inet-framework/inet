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

#ifndef __INET_STATISTICOSGVISUALIZER_H
#define __INET_STATISTICOSGVISUALIZER_H

#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Node>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

class INET_API StatisticOsgVisualizer : public StatisticVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class StatisticOsgVisualization : public StatisticVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        StatisticOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int moduleId, simsignal_t signal, const char *unit);
    };

  protected:
    NetworkNodeOsgVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override;
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override;

  public:
    virtual ~StatisticOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override { return nullptr;}
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICOSGVISUALIZER_H

