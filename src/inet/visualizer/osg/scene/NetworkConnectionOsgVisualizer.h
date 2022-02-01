//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKCONNECTIONOSGVISUALIZER_H
#define __INET_NETWORKCONNECTIONOSGVISUALIZER_H

#include "inet/visualizer/base/NetworkConnectionVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API NetworkConnectionOsgVisualizer : public NetworkConnectionVisualizerBase
{
  protected:
    virtual void initialize(int stage) override;

    virtual void createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode) override;
};

} // namespace visualizer

} // namespace inet

#endif

