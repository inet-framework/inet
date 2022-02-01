//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211CANVASVISUALIZER_H
#define __INET_IEEE80211CANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/LabeledIconFigure.h"
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211CanvasVisualizer : public Ieee80211VisualizerBase
{
  protected:
    class INET_API Ieee80211CanvasVisualization : public Ieee80211Visualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        LabeledIconFigure *figure = nullptr;

      public:
        Ieee80211CanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, LabeledIconFigure *figure, int networkNodeId, int interfaceId);
        virtual ~Ieee80211CanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power) override;
    virtual void addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
    virtual void removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
};

} // namespace visualizer

} // namespace inet

#endif

