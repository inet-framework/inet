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

#ifndef __INET_LINKBREAKVISUALIZERBASE_H
#define __INET_LINKBREAKVISUALIZERBASE_H

#include "inet/common/PatternMatcher.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API LinkBreakVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API LinkBreakVisualization {
      public:
        const int transmitterModuleId;
        const int receiverModuleId;
        mutable simtime_t breakSimulationTime;
        mutable double breakAnimationTime;
        mutable double breakRealTime;

      public:
        LinkBreakVisualization(int transmitterModuleId, int receiverModuleId, simtime_t breakSimulationTime, double breakAnimationTime, double breakRealTime);
        virtual ~LinkBreakVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    PatternMatcher nodeMatcher;
    const char *icon = nullptr;
    double iconTintAmount = NaN;
    cFigure::Color iconTintColor;
    const char *fadeOutMode = nullptr;
    double fadeOutHalfLife = NaN;
    //@}

    std::map<std::pair<int, int>, const LinkBreakVisualization *> linkBreakVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG) override;

    virtual void setPosition(cModule *node, const Coord& position) const = 0;
    virtual void setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const = 0;
    virtual const LinkBreakVisualization *createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const = 0;
    virtual void addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization);
    virtual void removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization);

    virtual cModule *findNode(MACAddress address);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_LINKBREAKVISUALIZERBASE_H

