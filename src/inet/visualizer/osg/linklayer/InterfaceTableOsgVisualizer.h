//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACETABLEOSGVISUALIZER_H
#define __INET_INTERFACETABLEOSGVISUALIZER_H

#include <osg/Node>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InterfaceTableOsgVisualizer : public InterfaceTableVisualizerBase
{
  protected:
    class INET_API InterfaceOsgVisualization : public InterfaceVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        InterfaceOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int networkNodeGateId, int interfaceId);
    };

  protected:
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

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

