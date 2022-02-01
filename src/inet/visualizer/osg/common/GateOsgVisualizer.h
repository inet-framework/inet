//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATEOSGVISUALIZER_H
#define __INET_GATEOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/GateVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

class INET_API GateOsgVisualizer : public GateVisualizerBase
{
  protected:
    class INET_API GateOsgVisualization : public GateVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        GateOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, queueing::IPacketGate *gate);
    };

  protected:
    // parameters
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual GateVisualization *createGateVisualization(queueing::IPacketGate *gate) const override;
    virtual void refreshGateVisualization(const GateVisualization *gateVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

