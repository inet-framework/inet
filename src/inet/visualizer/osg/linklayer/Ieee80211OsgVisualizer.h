//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OSGVISUALIZER_H
#define __INET_IEEE80211OSGVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211OsgVisualizer : public Ieee80211VisualizerBase
{
  protected:
    class INET_API Ieee80211OsgVisualization : public Ieee80211Visualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        Ieee80211OsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int interfaceId);
    };

  protected:
    ModuleRefByPar<NetworkNodeOsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power) override;
    virtual void addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
    virtual void removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

