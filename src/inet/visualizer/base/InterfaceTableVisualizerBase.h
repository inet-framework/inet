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

#include "inet/common/StringFormat.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API InterfaceTableVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API InterfaceVisualization {
      public:
        const int networkNodeId = -1;
        const int networkNodeGateId = -1;
        const int interfaceId = -1;

      public:
        InterfaceVisualization(int networkNodeId, int networkNodeGateId, int interfaceId);
        virtual ~InterfaceVisualization() {}
    };

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const InterfaceEntry *interfaceEntry = nullptr;

      public:
        DirectiveResolver(const InterfaceEntry *interfaceEntry) : interfaceEntry(interfaceEntry) { }

        virtual const char *resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayInterfaceTables = false;
    bool displayWiredInterfacesAtConnections = false;
    bool displayBackground = false;
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    StringFormat format;
    Placement placementHint;
    double placementPriority;
    cFigure::Font font;
    cFigure::Color textColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    //@}

    std::map<std::pair<int, int>, const InterfaceVisualization *> interfaceVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual cModule *getNetworkNode(const InterfaceVisualization *interfaceVisualization);
    virtual cGate *getOutputGate(cModule *networkNode, InterfaceEntry *interfaceEntry);
    virtual cGate *getOutputGate(const InterfaceVisualization *interfaceVisualization);

    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry) = 0;
    virtual const InterfaceVisualization *getInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry);
    virtual void addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization);
    virtual void addAllInterfaceVisualizations();
    virtual void removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization);
    virtual void removeAllInterfaceVisualizations();
    virtual void updateAllInterfaceVisualizations();

    virtual std::string getVisualizationText(const InterfaceEntry *interfaceEntry);
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const InterfaceEntry *interfaceEntry) = 0;

  public:
    virtual ~InterfaceTableVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INTERFACETABLEVISUALIZERBASE_H

