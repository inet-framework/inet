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

#ifndef __INET_PATHVISUALIZERBASE_H
#define __INET_PATHVISUALIZERBASE_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/PatternMatcher.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API Path {
      public:
        mutable double offset = NaN;
        mutable simtime_t lastUsage = simTime();
        const std::vector<int> moduleIds;

      public:
        Path(const std::vector<int>& path);
        virtual ~Path() {}
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    inet::PatternMatcher packetNameMatcher;
    cFigure::Color lineColor;
    double lineWidth = NaN;
    double opacityHalfLife = NaN;
    //@}

    /**
     * Maps packet to module vector.
     */
    std::map<int, std::vector<int>> incompletePaths;
    /**
     * Maps source/destination modules to multiple paths between them.
     */
    std::multimap<std::pair<int, int>, const Path *> paths;
    /**
     * Maps nodes to the number of paths that go through it.
     */
    std::map<int, int> numPaths;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual bool isPathEnd(cModule *module) const = 0;
    virtual bool isPathElement(cModule *module) const = 0;

    virtual const Path *createPath(const std::vector<int>& path) const = 0;
    virtual void setAlpha(const Path *path, double alpha) const = 0;
    virtual void setPosition(cModule *node, const Coord& position) const = 0;

    virtual const Path *getPath(std::pair<int, int> sourceAndDestination, const std::vector<int>& path);
    virtual void addPath(std::pair<int, int> sourceAndDestination, const Path *path);
    virtual void removePath(std::pair<int, int> sourceAndDestination, const Path *path);

    virtual const std::vector<int> *getIncompletePath(int treeId);
    virtual void addToIncompletePath(int treeId, cModule *module);
    virtual void removeIncompletePath(int treeId);

    virtual void updateOffsets();
    virtual void updatePositions();
    virtual void updatePath(const std::vector<int>& path);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PATHVISUALIZERBASE_H

