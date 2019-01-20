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

#ifndef __INET_TRANSPORTCONNECTIONVISUALIZERBASE_H
#define __INET_TRANSPORTCONNECTIONVISUALIZERBASE_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/PortFilter.h"

#ifdef WITH_TCP_INET
#include "inet/transportlayer/tcp/TcpConnection.h"
#else
namespace inet { namespace tcp { class TcpConnection; } }
#endif // WITH_TCP_INET

namespace inet {

namespace visualizer {

class INET_API TransportConnectionVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API TransportConnectionVisualization {
      public:
        int sourceModuleId = -1;
        int destinationModuleId = -1;
        int count = -1;

      public:
        TransportConnectionVisualization(int sourceModuleId, int destinationModuleId, int count);
        virtual ~TransportConnectionVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayTransportConnections = false;
    NetworkNodeFilter sourceNodeFilter;
    PortFilter sourcePortFilter;
    NetworkNodeFilter destinationNodeFilter;
    PortFilter destinationPortFilter;
    const char *icon = nullptr;
    ColorSet iconColorSet;
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    Placement placementHint;
    double placementPriority;
    //@}

    std::vector<const TransportConnectionVisualization *> connectionVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const = 0;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization);
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization);
    virtual void removeAllConnectionVisualizations();

  public:
    virtual ~TransportConnectionVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_TRANSPORTCONNECTIONVISUALIZERBASE_H

