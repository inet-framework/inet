//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RATECANVASVISUALIZER_H
#define __INET_IEEE80211RATECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/visualizer/base/Ieee80211RateVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211RateCanvasVisualizer : public Ieee80211RateVisualizerBase
{
  protected:
    class INET_API Ieee80211RateCanvasVisualization : public Ieee80211RateVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        cGroupFigure *figure = nullptr;
        cFigure::Rectangle bounds = cFigure::Rectangle(0, 0, 0, 0);
        std::string title;

      public:
        Ieee80211RateCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cGroupFigure *figure, int networkNodeId, int interfaceId);
        virtual ~Ieee80211RateCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual Ieee80211RateVisualization *createRateVisualization(cModule *networkNode, NetworkInterface *networkInterface) override;
    virtual void addRateVisualization(Ieee80211RateVisualization *rateVisualization) override;
    virtual void removeRateVisualization(Ieee80211RateVisualization *rateVisualization) override;

    // Rebuilds the bar chart figure content from the current rate entries.
    virtual void refreshChart(Ieee80211RateCanvasVisualization *canvasVisualization) const;
};

} // namespace visualizer

} // namespace inet

#endif
