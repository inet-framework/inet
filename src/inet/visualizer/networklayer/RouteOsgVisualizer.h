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

#ifndef __INET_ROUTEOSGVISUALIZER_H
#define __INET_ROUTEOSGVISUALIZER_H

#include "inet/visualizer/base/RouteVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RouteOsgVisualizer : public RouteVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API OsgRoute : public Route {
      public:
        osg::Node *node = nullptr;

      public:
        OsgRoute(const std::vector<int>& path, osg::Node *node);
        virtual ~OsgRoute();
    };

  protected:
    virtual void addRoute(std::pair<int, int> sourceAndDestination, const Route *route) override;
    virtual void removeRoute(std::pair<int, int> sourceAndDestination, const Route *route) override;

    virtual const Route *createRoute(const std::vector<int>& path) const override;
    virtual void setAlpha(const Route *route, double alpha) const override;
    virtual void setPosition(cModule *node, const Coord& position) const override;

#else // ifdef WITH_OSG

  protected:
    virtual const Route *createRoute(const std::vector<int>& path) const override { return RouteVisualizerBase::createRoute(path); }
    virtual void setAlpha(const Route *route, double alpha) const override {}
    virtual void setPosition(cModule *node, const Coord& position) const override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTEOSGVISUALIZER_H

