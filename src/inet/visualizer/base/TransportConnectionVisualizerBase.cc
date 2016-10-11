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
#include "inet/transportlayer/tcp/TCP.h"
#include "inet/transportlayer/tcp/TCPConnection.h"
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"

namespace inet {

namespace visualizer {

TransportConnectionVisualizerBase::TransportConnectionVisualization::TransportConnectionVisualization(int sourceModuleId, int destinationModuleId, int count) :
    sourceModuleId(sourceModuleId),
    destinationModuleId(destinationModuleId),
    count(count)
{
}

void TransportConnectionVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(inet::tcp::TCP::tcpConnectionAddedSignal, this);
        nodeMatcher.setPattern(par("nodeFilter"), false, true, true);
    }
}

void TransportConnectionVisualizerBase::addConnectionVisualization(const TransportConnectionVisualization *connection)
{
    connections.push_back(connection);
}

void TransportConnectionVisualizerBase::removeConnectionVisualization(const TransportConnectionVisualization *connection)
{
    connections.erase(std::remove(connections.begin(), connections.end(), connection), connections.end());
}

void TransportConnectionVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == inet::tcp::TCP::tcpConnectionAddedSignal) {
        auto tcpConnection = check_and_cast<inet::tcp::TCPConnection *>(object);
        L3AddressResolver resolver;
        auto source = resolver.findHostWithAddress(tcpConnection->localAddr);
        auto destination = resolver.findHostWithAddress(tcpConnection->remoteAddr);
        if (source != nullptr && nodeMatcher.matches(source->getFullPath().c_str()) &&
            destination != nullptr && nodeMatcher.matches(destination->getFullPath().c_str()))
        {
            addConnectionVisualization(createConnectionVisualization(source, destination, tcpConnection));
        }
    }
}

} // namespace visualizer

} // namespace inet

