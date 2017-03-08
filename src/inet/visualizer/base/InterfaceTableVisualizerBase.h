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

#ifndef __INET_INTERFACETABLEVISUALIZERBASE_H
#define __INET_INTERFACETABLEVISUALIZERBASE_H

#include "inet/common/PatternMatcher.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API InterfaceTableVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API InterfaceVisualization {
      public:
        const int networkNodeId = -1;
        const int interfaceId = -1;

      public:
        InterfaceVisualization(int networkNodeId, int interfaceId);
        virtual ~InterfaceVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    PatternMatcher nodeMatcher;
    PatternMatcher interfaceMatcher;
    const char *content = nullptr;
    cFigure::Color fontColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    //@}

    std::map<std::pair<int, int>, const InterfaceVisualization *> interfaceVisualizations;

  protected:
    virtual void initialize(int stage) override;

    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry) = 0;
    virtual const InterfaceVisualization *getInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry);
    virtual void addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization);
    virtual void removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization);

    virtual std::string getVisualizationText(const InterfaceEntry *interfaceEntry);
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const InterfaceEntry *interfaceEntry) = 0;

  public:
    virtual ~InterfaceTableVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INTERFACETABLEVISUALIZERBASE_H

