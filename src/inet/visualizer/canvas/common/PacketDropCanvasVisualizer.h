//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDROPCANVASVISUALIZER_H
#define __INET_PACKETDROPCANVASVISUALIZER_H

#include "inet/common/figures/LabeledIconFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PacketDropCanvasVisualizer : public PacketDropVisualizerBase
{
  protected:
    class INET_API PacketDropCanvasVisualization : public PacketDropVisualization {
      public:
        LabeledIconFigure *figure;

      public:
        PacketDropCanvasVisualization(LabeledIconFigure *figure, const PacketDrop *packetDrop);
        virtual ~PacketDropCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *packetDropGroup = nullptr;
    double dx = NaN;
    double dy = NaN;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const PacketDropVisualization *createPacketDropVisualization(PacketDrop *packetDrop) const override;
    virtual void addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization) override;
    virtual void setAlpha(const PacketDropVisualization *packetDrop, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

