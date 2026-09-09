//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211VSGVISUALIZER_H
#define __INET_IEEE80211VSGVISUALIZER_H

#include <string>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211VsgVisualizer : public Ieee80211VisualizerBase
{
  protected:
    class INET_API Ieee80211VsgVisualization : public Ieee80211Visualization {
      public:
        NetworkNodeVsgVisualization *networkNodeVisualization = nullptr;
        // Annotation container; holds a label (SSID text) and/or a small colored sphere
        // approximating the association-strength icon.
        // TODO: textured icon — replace sphere+label with a textured-quad billboard once
        //       VsgUtils gains createTexturedBillboard() support (currently no VSG
        //       textured-billboard helper exists).
        ::vsg::ref_ptr<::vsg::Group> node;

      public:
        Ieee80211VsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization,
                                  ::vsg::ref_ptr<::vsg::Group> node,
                                  int networkNodeId, int interfaceId);
    };

  protected:
    ModuleRefByPar<NetworkNodeVsgVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power) override;
    virtual void addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
    virtual void removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
};

} // namespace visualizer

} // namespace inet

#endif
