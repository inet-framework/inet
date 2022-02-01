//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEVISUALIZERBASE_H
#define __INET_NETWORKNODEVISUALIZERBASE_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeVisualizerBase : public VisualizerBase, public cListener
{
  public:
    class INET_API NetworkNodeVisualization {
      public:
        const cModule *networkNode;

      public:
        NetworkNodeVisualization(const cModule *networkNode) : networkNode(networkNode) {}
        virtual ~NetworkNodeVisualization() {}
    };

  protected:
    NetworkNodeFilter nodeFilter;
    double annotationSpacing;
    double placementPenalty;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual NetworkNodeVisualization *createNetworkNodeVisualization(cModule *networkNode) const = 0;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) = 0;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) = 0;
    virtual void destroyNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) = 0;

  public:
    virtual NetworkNodeVisualization *findNetworkNodeVisualization(const cModule *networkNode) const = 0;
    virtual NetworkNodeVisualization *getNetworkNodeVisualization(const cModule *networkNode) const;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

