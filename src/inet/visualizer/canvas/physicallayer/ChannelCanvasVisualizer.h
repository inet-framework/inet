//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHANNELCANVASVISUALIZER_H
#define __INET_CHANNELCANVASVISUALIZER_H

#include "inet/common/figures/LabeledLineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/ChannelVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API ChannelCanvasVisualizer : public ChannelVisualizerBase
{
  protected:
    class INET_API ChannelCanvasVisualization : public ChannelVisualization {
      public:
        LabeledLineFigure *figure = nullptr;

      public:
        ChannelCanvasVisualization(LabeledLineFigure *figure, int sourceModuleId, int destinationModuleId);
        virtual ~ChannelCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *channelActivityGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const ChannelVisualization *createChannelVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void addChannelVisualization(std::pair<int, int> sourceAndDestination, const ChannelVisualization *channelVisualization) override;
    virtual void removeChannelVisualization(const ChannelVisualization *channelVisualization) override;
    virtual void setAlpha(const ChannelVisualization *channelVisualization, double alpha) const override;
    virtual void refreshChannelVisualization(const ChannelVisualization *channelVisualization, cPacket *packet) override;
};

} // namespace visualizer

} // namespace inet

#endif

