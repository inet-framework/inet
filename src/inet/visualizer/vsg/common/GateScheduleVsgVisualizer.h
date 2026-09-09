//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATESCHEDULEVSGVISUALIZER_H
#define __INET_GATESCHEDULEVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/GateScheduleVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API GateScheduleVsgVisualizer : public GateScheduleVisualizerBase
{
  protected:
    class INET_API GateVsgVisualization : public GateVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; rebuilt when gate state changes
        mutable std::string lastText;        // cached label text to avoid unnecessary rebuilds

      public:
        GateVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, queueing::IPacketGate *gate);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual GateVisualization *createGateVisualization(queueing::IPacketGate *gate) const override;
    virtual void refreshGateVisualization(const GateVisualization *gateVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif
