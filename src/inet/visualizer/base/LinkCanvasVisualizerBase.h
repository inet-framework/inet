//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_LINKCANVASVISUALIZERBASE_H
#define __INET_LINKCANVASVISUALIZERBASE_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/LinkVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkCanvasVisualizerBase : public LinkVisualizerBase
{
  protected:
    class INET_API CanvasLink : public Link {
      public:
        cLineFigure *figure = nullptr;

      public:
        CanvasLink(cLineFigure *figure, int sourceModuleId, int destinationModuleId);
        virtual ~CanvasLink();
    };

  protected:
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *linkGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void addLink(std::pair<int, int> sourceAndDestination, const Link *link) override;
    virtual void removeLink(const Link *link) override;

    virtual const Link *createLink(cModule *source, cModule *destination) const override;
    virtual void setAlpha(const Link *link, double alpha) const override;
    virtual void setPosition(cModule *node, const Coord& position) const override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_LINKCANVASVISUALIZERBASE_H

