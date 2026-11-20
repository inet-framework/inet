//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTCONNECTIONVISUALIZERBASE_H
#define __INET_TRANSPORTCONNECTIONVISUALIZERBASE_H

#include <map>

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/PortFilter.h"

#ifdef INET_WITH_TCP_INET
#include "inet/transportlayer/tcp/TcpConnection.h"
#else
namespace inet { namespace tcp { class TcpConnection; } }
#endif // INET_WITH_TCP_INET

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
        int index = -1; // selects the color and the letter, unique among the displayed connections

      public:
        TransportConnectionVisualization(int sourceModuleId, int destinationModuleId, int count, int index = -1);
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
    std::map<int, const TransportConnectionVisualization *> connectionIdToVisualization; // TcpConnection module id to its visualization

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const = 0;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization);
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization);
    virtual void removeAllConnectionVisualizations();

    /**
     * Returns the lowest index that no displayed connection has.
     */
    virtual int getFreeConnectionIndex() const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

