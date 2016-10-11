//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
    class INET_API CanvasLinkBreak : public LinkBreakVisualization {
      public:
        cIconFigure *figure = nullptr;

      public:
        CanvasLinkBreak(cIconFigure *figure, int transmitterModuleId, int receiverModuleId, simtime_t breakSimulationTime, double breakAnimationTime, double breakRealTime);
        virtual ~CanvasLinkBreak() { delete figure; }
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *linkBreakGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void setPosition(cModule *node, const Coord& position) const override;
    virtual void setAlpha(const LinkBreakVisualization *linkBreak, double alpha) const override;
    virtual const LinkBreakVisualization *createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const override;
    virtual void addLinkBreakVisualization(const LinkBreakVisualization *linkBreak) override;
    virtual void removeLinkBreakVisualization(const LinkBreakVisualization *linkBreak) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_LINKBREAKCANVASVISUALIZER_H

