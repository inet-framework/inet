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

#ifndef __INET_IEEE80211OSGVISUALIZER_H
#define __INET_IEEE80211OSGVISUALIZER_H

#include "inet/visualizer/base/Ieee80211VisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211OsgVisualizer : public Ieee80211VisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API Ieee80211OsgVisualization : public Ieee80211Visualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        Ieee80211OsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int interfaceId);
    };

  protected:
    NetworkNodeOsgVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry, std::string ssid, W power) override;
    virtual void addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;
    virtual void removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization) override;

  public:
    virtual ~Ieee80211OsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry, std::string ssid, W power) override { return nullptr; }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_IEEE80211OSGVISUALIZER_H

