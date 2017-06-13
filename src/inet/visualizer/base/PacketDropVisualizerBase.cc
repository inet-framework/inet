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
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/queue/PassiveQueueBase.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/common/NotifierConsts.h"


namespace inet {

namespace visualizer {

PacketDropVisualizerBase::PacketDropVisualization::PacketDropVisualization(int moduleId, const cPacket *packet, const Coord& position) :
    moduleId(moduleId),
    packet(packet),
    position(position)
{
}

PacketDropVisualizerBase::PacketDropVisualization::~PacketDropVisualization()
{
    delete packet;
}

PacketDropVisualizerBase::~PacketDropVisualizerBase()
{
    for (auto packetDrop : packetDropVisualizations)
        delete packetDrop->packet;
    if (displayPacketDrops)
        unsubscribe();
}

void PacketDropVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayPacketDrops = par("displayPacketDrops");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setPattern(par("packetFilter"));
        icon = par("icon");
        iconTintAmount = par("iconTintAmount");
        if (iconTintAmount != 0)
            iconTintColor = cFigure::Color(par("iconTintColor"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        if (displayPacketDrops)
            subscribe();
    }
}

void PacketDropVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
        removeAllPacketDropVisualizations();
    }
}

void PacketDropVisualizerBase::refreshDisplay() const
{
    AnimationPosition currentAnimationPosition;
    std::vector<const PacketDropVisualization *> removedPacketDropVisualizations;
    for (auto packetDrop : packetDropVisualizations) {
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentAnimationPosition.getSimulationTime() - packetDrop->packetDropAnimationPosition.getSimulationTime()).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationPosition.getAnimationTime() - packetDrop->packetDropAnimationPosition.getAnimationTime();
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentAnimationPosition.getRealTime() - packetDrop->packetDropAnimationPosition.getRealTime();
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        if (delta > fadeOutTime)
            removedPacketDropVisualizations.push_back(packetDrop);
        else
            setAlpha(packetDrop, 1 - delta / fadeOutTime);
    }
    for (auto packetDrop : removedPacketDropVisualizations) {
        const_cast<PacketDropVisualizerBase *>(this)->removePacketDropVisualization(packetDrop);
        delete packetDrop;
    }
}

void PacketDropVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetFromLowerDroppedSignal, this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetFromUpperDroppedSignal, this);
    subscriptionModule->subscribe(PassiveQueueBase::dropPkByQueueSignal, this);
#ifdef WITH_ETHERNET
    subscriptionModule->subscribe(EtherMACBase::dropPkIfaceDownSignal, this);
    subscriptionModule->subscribe(EtherMACBase::dropPkFromHLIfaceDownSignal, this);
#endif // WITH_ETHERNET
    subscriptionModule->subscribe(NF_PACKET_DROP, this);

}

void PacketDropVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetFromLowerDroppedSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetFromUpperDroppedSignal, this);
        subscriptionModule->unsubscribe(PassiveQueueBase::dropPkByQueueSignal, this);
#ifdef WITH_ETHERNET
        subscriptionModule->unsubscribe(EtherMACBase::dropPkIfaceDownSignal, this);
        subscriptionModule->unsubscribe(EtherMACBase::dropPkFromHLIfaceDownSignal, this);
#endif // WITH_ETHERNET
        subscriptionModule->unsubscribe(NF_PACKET_DROP, this);
    }
}

void PacketDropVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == LayeredProtocolBase::packetFromLowerDroppedSignal || signal == LayeredProtocolBase::packetFromUpperDroppedSignal
            || signal == PassiveQueueBase::dropPkByQueueSignal
#ifdef WITH_ETHERNET
            || signal == EtherMACBase::dropPkIfaceDownSignal || signal == EtherMACBase::dropPkFromHLIfaceDownSignal
#endif // WITH_ETHERNET
            ) {
        auto packet = check_and_cast<cPacket *>(object);
        if (packetFilter.matches(packet))
            addPacketDropVisualization(createPacketDropVisualization(check_and_cast<cModule*>(source), packet->dup()));
    }
    else
        throw cRuntimeError("Unknown signal");
}

void PacketDropVisualizerBase::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    packetDropVisualizations.push_back(packetDropVisualization);
}

void PacketDropVisualizerBase::removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    packetDropVisualizations.erase(std::remove(packetDropVisualizations.begin(), packetDropVisualizations.end(), packetDropVisualization), packetDropVisualizations.end());
}

void PacketDropVisualizerBase::removeAllPacketDropVisualizations()
{
    for (auto packetDropVisualization : packetDropVisualizations) {
        removePacketDropVisualization(packetDropVisualization);
        delete packetDropVisualization;
    }
}

} // namespace visualizer

} // namespace inet

