//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKCANVASVISUALIZERBASE_H
#define __INET_LINKCANVASVISUALIZERBASE_H

#include "inet/common/figures/LabeledLineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/LinkVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkCanvasVisualizerBase : public LinkVisualizerBase
{
  protected:
    class INET_API LinkCanvasVisualization : public LinkVisualization {
      public:
        LabeledLineFigure *figure = nullptr;

      public:
        LinkCanvasVisualization(LabeledLineFigure *figure, int sourceModuleId, int destinationModuleId);
        virtual ~LinkCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *linkGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization) override;
    virtual void removeLinkVisualization(const LinkVisualization *linkVisualization) override;
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const override;
    virtual void refreshLinkVisualization(const LinkVisualization *linkVisualization, cPacket *packet) override;
};

} // namespace visualizer

} // namespace inet

#endif

