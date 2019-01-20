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

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#ifdef WITH_TCP_INET
#include "inet/transportlayer/tcp/Tcp.h"
#endif // WITH_TCP_INET
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"

namespace inet {

namespace visualizer {

TransportConnectionVisualizerBase::TransportConnectionVisualization::TransportConnectionVisualization(int sourceModuleId, int destinationModuleId, int count) :
    sourceModuleId(sourceModuleId),
    destinationModuleId(destinationModuleId),
    count(count)
{
}

TransportConnectionVisualizerBase::~TransportConnectionVisualizerBase()
{
    if (displayTransportConnections)
        unsubscribe();
}

void TransportConnectionVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayTransportConnections = par("displayTransportConnections");
        sourceNodeFilter.setPattern(par("sourceNodeFilter"));
        sourcePortFilter.setPattern(par("sourcePortFilter"));
        destinationNodeFilter.setPattern(par("destinationNodeFilter"));
        destinationPortFilter.setPattern(par("destinationPortFilter"));
        icon = par("icon");
        iconColorSet.parseColors(par("iconColor"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        if (displayTransportConnections)
            subscribe();
    }
}

void TransportConnectionVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "sourceNodeFilter"))
            sourceNodeFilter.setPattern(par("sourceNodeFilter"));
        else if (!strcmp(name, "sourcePortFilter"))
            sourcePortFilter.setPattern(par("sourcePortFilter"));
        else if (!strcmp(name, "destinationNodeFilter"))
            sourceNodeFilter.setPattern(par("destinationNodeFilter"));
        else if (!strcmp(name, "destinationPortFilter"))
            sourcePortFilter.setPattern(par("destinationPortFilter"));
        removeAllConnectionVisualizations();
    }
}

void TransportConnectionVisualizerBase::subscribe()
{
#ifdef WITH_TCP_INET
    visualizationSubjectModule->subscribe(inet::tcp::Tcp::tcpConnectionAddedSignal, this);
#endif // WITH_TCP_INET
}

void TransportConnectionVisualizerBase::unsubscribe()
{
#ifdef WITH_TCP_INET
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = getModuleFromPar<cModule>(par("visualizationSubjectModule"), this, false);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(inet::tcp::Tcp::tcpConnectionAddedSignal, this);
#endif // WITH_TCP_INET
}

void TransportConnectionVisualizerBase::addConnectionVisualization(const TransportConnectionVisualization *connection)
{
    connectionVisualizations.push_back(connection);
}

void TransportConnectionVisualizerBase::removeConnectionVisualization(const TransportConnectionVisualization *connection)
{
    connectionVisualizations.erase(std::remove(connectionVisualizations.begin(), connectionVisualizations.end(), connection), connectionVisualizations.end());
}

void TransportConnectionVisualizerBase::removeAllConnectionVisualizations()
{
    std::vector<const TransportConnectionVisualization *> removedConnectionVisualizations;
    for (auto it : connectionVisualizations)
        removedConnectionVisualizations.push_back(it);
    for (auto it : removedConnectionVisualizations) {
        removeConnectionVisualization(it);
        delete it;
    }
}

void TransportConnectionVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
#ifdef WITH_TCP_INET
    Enter_Method_Silent();
    if (signal == inet::tcp::Tcp::tcpConnectionAddedSignal) {
        auto tcpConnection = check_and_cast<inet::tcp::TcpConnection *>(object);
        L3AddressResolver resolver;
        auto source = resolver.findHostWithAddress(tcpConnection->localAddr);
        auto destination = resolver.findHostWithAddress(tcpConnection->remoteAddr);
        if (source != nullptr && sourceNodeFilter.matches(source) &&
            destination != nullptr && destinationNodeFilter.matches(destination) &&
            sourcePortFilter.matches(tcpConnection->localPort) &&
            destinationPortFilter.matches(tcpConnection->remotePort))
        {
            addConnectionVisualization(createConnectionVisualization(source, destination, tcpConnection));
        }
    }
    else
        throw cRuntimeError("Unknown signal");
#endif // WITH_TCP_INET
}

} // namespace visualizer

} // namespace inet

