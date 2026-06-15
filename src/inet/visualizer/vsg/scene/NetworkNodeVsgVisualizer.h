//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEVSGVISUALIZER_H
#define __INET_NETWORKNODEVSGVISUALIZER_H

#include <map>

#include <vsg/core/ref_ptr.h>

#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeVsgVisualizer : public NetworkNodeVisualizerBase
{
  protected:
    bool displayModuleName = true;
    std::map<int, ::vsg::ref_ptr<NetworkNodeVsgVisualization>> networkNodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual NetworkNodeVsgVisualization *createNetworkNodeVisualization(cModule *networkNode) const override;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void destroyNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override {}

  public:
    virtual ~NetworkNodeVsgVisualizer();
    virtual NetworkNodeVsgVisualization *findNetworkNodeVisualization(const cModule *networkNode) const override;
    virtual NetworkNodeVsgVisualization *getNetworkNodeVisualization(const cModule *networkNode) const override;
};

} // namespace visualizer

} // namespace inet

#endif
