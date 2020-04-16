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

#ifndef __INET_INFOOSGVISUALIZER_H
#define __INET_INFOOSGVISUALIZER_H

#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/InfoVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API InfoOsgVisualizer : public InfoVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API InfoOsgVisualization : public InfoVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Geode *node = nullptr;

      public:
        InfoOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *figure, int moduleId);
    };

  protected:
    // parameters
    NetworkNodeOsgVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override;
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override;

  public:
    virtual ~InfoOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual InfoVisualization *createInfoVisualization(cModule *module) const override { return nullptr; }
    virtual void refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INFOOSGGVISUALIZER_H

