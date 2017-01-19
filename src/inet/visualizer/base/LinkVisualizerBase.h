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

#ifndef __INET_LINKVISUALIZERBASE_H
#define __INET_LINKVISUALIZERBASE_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/PatternMatcher.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API Link {
      public:
        mutable simtime_t lastUsage = simTime();
        const int sourceModuleId;
        const int destinationModuleId;

      public:
        Link(int sourceModuleId, int destinationModuleId);
        virtual ~Link() {}
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    PatternMatcher packetNameMatcher;
    cFigure::Color lineColor;
    double lineWidth = NaN;
    cFigure::LineStyle lineStyle;
    double opacityHalfLife = NaN;
    //@}

    /**
     * Maps packet to last module.
     */
    std::map<int, int> lastModules;
    /**
     * Maps source/destination module to link.
     */
    std::map<std::pair<int, int>, const Link *> links;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual bool isLinkEnd(cModule *module) const = 0;

    virtual const Link *createLink(cModule *source, cModule *destination) const = 0;
    virtual void setAlpha(const Link *link, double alpha) const = 0;
    virtual void setPosition(cModule *node, const Coord& position) const = 0;

    virtual const Link *getLink(std::pair<int, int> link);
    virtual void addLink(std::pair<int, int> sourceAndDestination, const Link *link);
    virtual void removeLink(const Link *link);

    virtual cModule *getLastModule(int treeId);
    virtual void setLastModule(int treeId, cModule *lastModule);
    virtual void removeLastModule(int treeId);

    virtual void updateLink(cModule *source, cModule *destination);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_LINKVISUALIZERBASE_H

