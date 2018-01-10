//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211VISUALIZERBASE_H
#define __INET_IEEE80211VISUALIZERBASE_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211VisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API Ieee80211Visualization {
      public:
        const int networkNodeId = -1;
        const int interfaceId = -1;

      public:
        Ieee80211Visualization(int networkNodeId, int interfaceId);
        virtual ~Ieee80211Visualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayAssociations = false;
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    double minPowerDbm = NaN;
    double maxPowerDbm = NaN;
    std::vector<std::string> icons;
    ColorSet iconColorSet;
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    bool showLabelNextToIcon = false;
    Placement placementHint;
    double placementPriority;
    //@}

    std::map<std::pair<int, int>, const Ieee80211Visualization *> ieee80211Visualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual Ieee80211Visualization *createIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power) = 0;
    virtual const Ieee80211Visualization *getIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface);
    virtual void addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization);
    virtual void removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization);
    virtual void removeAllIeee80211Visualizations();

    virtual std::string getIcon(W power) const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

