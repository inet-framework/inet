//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINKBREAKCANVASVISUALIZER_H
#define __INET_LINKBREAKCANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/LinkBreakVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkBreakCanvasVisualizer : public LinkBreakVisualizerBase
{
  protected:
    class INET_API LinkBreakCanvasVisualization : public LinkBreakVisualization {
      public:
        cIconFigure *figure = nullptr;

      public:
        LinkBreakCanvasVisualization(cIconFigure *figure, int transmitterModuleId, int receiverModuleId);
        virtual ~LinkBreakCanvasVisualization() { delete figure; }
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *linkBreakGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const LinkBreakVisualization *createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const override;
    virtual void addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization) override;
    virtual void removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization) override;
    virtual void setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

