//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INFOCANVASVISUALIZER_H
#define __INET_INFOCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InfoCanvasVisualizer : public InfoVisualizerBase
{
  protected:
    class INET_API InfoCanvasVisualization : public InfoVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        BoxedLabelFigure *figure = nullptr;

      public:
        InfoCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int moduleId);
        virtual ~InfoCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override;
    virtual void addInfoVisualization(const InfoVisualization *infoVisualization) override;
    virtual void removeInfoVisualization(const InfoVisualization *infoVisualization) override;
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override;
};

} // namespace visualizer

} // namespace inet

#endif

