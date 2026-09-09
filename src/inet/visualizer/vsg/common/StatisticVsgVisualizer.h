//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICVSGVISUALIZER_H
#define __INET_STATISTICVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/StatisticVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API StatisticVsgVisualizer : public StatisticVisualizerBase
{
  protected:
    class INET_API StatisticVsgVisualization : public StatisticVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; holds the current label
        mutable std::string lastText;       // avoid rebuilding the label when unchanged

      public:
        StatisticVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int moduleId, simsignal_t signal, const char *unit);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

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
