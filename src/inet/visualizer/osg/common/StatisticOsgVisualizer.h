//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICOSGVISUALIZER_H
#define __INET_STATISTICOSGVISUALIZER_H

#include <osg/Node>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API StatisticOsgVisualizer : public StatisticVisualizerBase
{
  protected:
    class INET_API StatisticOsgVisualization : public StatisticVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        StatisticOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int moduleId, simsignal_t signal, const char *unit);
    };

  protected:
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) override;
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

