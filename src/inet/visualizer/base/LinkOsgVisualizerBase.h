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

#ifndef __INET_LINKOSGVISUALIZERBASE_H
#define __INET_LINKOSGVISUALIZERBASE_H

#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/LinkVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkOsgVisualizerBase : public LinkVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API LinkOsgVisualization : public LinkVisualization {
      public:
        inet::osg::LineNode *node = nullptr;

      public:
        LinkOsgVisualization(inet::osg::LineNode *node, int sourceModuleId, int destinationModuleId);
        virtual ~LinkOsgVisualization();
    };

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization) override;
    virtual void removeLinkVisualization(const LinkVisualization *linkVisualization) override;
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const override;

  public:
    virtual ~LinkOsgVisualizerBase();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override { return nullptr; }
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_LINKOSGVISUALIZERBASE_H

