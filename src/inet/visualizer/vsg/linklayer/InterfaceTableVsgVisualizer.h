//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACETABLEVSGVISUALIZER_H
#define __INET_INTERFACETABLEVSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InterfaceTableVsgVisualizer : public InterfaceTableVisualizerBase
{
  protected:
    class INET_API InterfaceVsgVisualization : public InterfaceVisualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        ::vsg::ref_ptr<::vsg::Group> node;  // annotation container; holds the current label
        mutable std::string lastText;       // avoid rebuilding the label when unchanged

      public:
        InterfaceVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int networkNodeId, int networkNodeGateId, int interfaceId);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface) override;
    virtual void addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const NetworkInterface *networkInterface) override;
};

} // namespace visualizer

} // namespace inet

#endif
