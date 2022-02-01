//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTCONNECTIONOSGVISUALIZER_H
#define __INET_TRANSPORTCONNECTIONOSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API TransportConnectionOsgVisualizer : public TransportConnectionVisualizerBase
{
  protected:
    class INET_API TransportConnectionOsgVisualization : public TransportConnectionVisualization {
      public:
        osg::Node *sourceNode = nullptr;
        osg::Node *destinationNode = nullptr;

      public:
        TransportConnectionOsgVisualization(osg::Node *sourceNode, osg::Node *destinationNode, int sourceModuleId, int destinationModuleId, int count);
    };

  protected:
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual osg::Node *createConnectionEndNode(tcp::TcpConnection *connectionVisualization) const;
    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const override;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

