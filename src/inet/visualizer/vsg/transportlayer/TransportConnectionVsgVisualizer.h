//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTCONNECTIONVSGVISUALIZER_H
#define __INET_TRANSPORTCONNECTIONVSGVISUALIZER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API TransportConnectionVsgVisualizer : public TransportConnectionVisualizerBase
{
  protected:
    class INET_API TransportConnectionVsgVisualization : public TransportConnectionVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Node> sourceNode;
        ::vsg::ref_ptr<::vsg::Node> destinationNode;

      public:
        TransportConnectionVsgVisualization(::vsg::ref_ptr<::vsg::Node> sourceNode, ::vsg::ref_ptr<::vsg::Node> destinationNode, int sourceModuleId, int destinationModuleId, int count);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual ::vsg::ref_ptr<::vsg::Node> createConnectionEndNode(tcp::TcpConnection *tcpConnection) const;
    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const override;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
};

} // namespace visualizer

} // namespace inet

#endif
