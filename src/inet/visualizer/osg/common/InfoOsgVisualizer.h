//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INFOOSGVISUALIZER_H
#define __INET_INFOOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API InfoOsgVisualizer : public InfoVisualizerBase
{
  protected:
    class INET_API InfoOsgVisualization : public InfoVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        InfoOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, int moduleId);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override;
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override;
};

} // namespace visualizer

} // namespace inet

#endif

