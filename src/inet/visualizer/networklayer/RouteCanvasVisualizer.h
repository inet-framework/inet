//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_ROUTECANVASVISUALIZER_H
#define __INET_ROUTECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/RouteVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RouteCanvasVisualizer : public RouteVisualizerBase
{
  protected:
    class INET_API CanvasRoute : public Route {
      public:
        cPolylineFigure *figure = nullptr;

      public:
        CanvasRoute(const std::vector<int>& path, cPolylineFigure *figure);
        virtual ~CanvasRoute();
    };

  protected:
    const CanvasProjection *canvasProjection = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void addRoute(std::pair<int, int> sourceAndDestination, const Route *route) override;
    virtual void removeRoute(std::pair<int, int> sourceAndDestination, const Route *route) override;

    virtual const Route *createRoute(const std::vector<int>& path) const override;
    virtual void setAlpha(const Route *route, double alpha) const override;
    virtual void setPosition(cModule *node, const Coord& position) const override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTECANVASVISUALIZER_H

