//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEOSGVISUALIZER_H
#define __INET_NETWORKNODEOSGVISUALIZER_H

#include <osg/ref_ptr>

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualization.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeOsgVisualizer : public NetworkNodeVisualizerBase
{
  protected:
    bool displayModuleName;
    std::map<int, osg::ref_ptr<NetworkNodeOsgVisualization>> networkNodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual NetworkNodeOsgVisualization *createNetworkNodeVisualization(cModule *networkNode) const override;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void destroyNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override {}

  public:
    virtual ~NetworkNodeOsgVisualizer();
    virtual NetworkNodeOsgVisualization *findNetworkNodeVisualization(const cModule *networkNode) const override;
    virtual NetworkNodeOsgVisualization *getNetworkNodeVisualization(const cModule *networkNode) const override;
};

} // namespace visualizer

} // namespace inet

#endif

