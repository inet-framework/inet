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

#ifndef __INET_PATHOSGVISUALIZERBASE_H
#define __INET_PATHOSGVISUALIZERBASE_H

#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathOsgVisualizerBase : public PathVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API OsgPath : public Path {
      public:
        osg::Node *node = nullptr;

      public:
        OsgPath(const std::vector<int>& path, osg::Node *node);
        virtual ~OsgPath();
    };

  protected:
    virtual void addPath(std::pair<int, int> sourceAndDestination, const Path *path) override;
    virtual void removePath(std::pair<int, int> sourceAndDestination, const Path *path) override;

    virtual const Path *createPath(const std::vector<int>& path) const override;
    virtual void setAlpha(const Path *path, double alpha) const override;
    virtual void setPosition(cModule *node, const Coord& position) const override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual const Path *createPath(const std::vector<int>& path) const override { return PathVisualizerBase::createPath(path); }
    virtual void setAlpha(const Path *path, double alpha) const override {}
    virtual void setPosition(cModule *node, const Coord& position) const override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PATHOSGVISUALIZERBASE_H

