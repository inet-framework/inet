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
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"

namespace inet {

namespace visualizer {

PacketDropVisualizerBase::PacketDropVisualization::PacketDropVisualization(int moduleId, cPacket *packet, simtime_t dropSimulationTime, double dropAnimationTime, int dropRealTime) :
    moduleId(moduleId),
    packet(packet),
    dropSimulationTime(dropSimulationTime),
    dropAnimationTime(dropAnimationTime),
    dropRealTime(dropRealTime)
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
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetFromLowerDroppedSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetFromUpperDroppedSignal, this);
    }
}

void PacketDropVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(IMobility::mobilityStateChangedSignal, this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetFromLowerDroppedSignal, this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetFromUpperDroppedSignal, this);
        packetNameMatcher.setPattern(par("packetNameFilter"), false, true, true);
        icon = par("icon");
        iconTintAmount = par("iconTintAmount");
        if (iconTintAmount != 0)
            iconTintColor = cFigure::Color(par("iconTintColor"));
        fadeOutMode = par("fadeOutMode");
        fadeOutHalfLife = par("fadeOutHalfLife");
    }
}

void PacketDropVisualizerBase::refreshDisplay() const
{
    auto currentSimulationTime = simTime();
    double currentAnimationTime = getSimulation()->getEnvir()->getAnimationTime();
    double currentRealTime = getRealTime();
    std::vector<const PacketDropVisualization *> removedPacketDrops;
    for (auto packetDrop : packetDropVisualizations) {
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentSimulationTime - packetDrop->dropSimulationTime).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationTime - packetDrop->dropAnimationTime;
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentRealTime - packetDrop->dropRealTime;
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        auto alpha = std::min(1.0, std::pow(2.0, -delta / fadeOutHalfLife));
        if (alpha < 0.01)
            removedPacketDrops.push_back(packetDrop);
        else
            setAlpha(packetDrop, alpha);
    }
    for (auto packetDrop : removedPacketDrops) {
        const_cast<PacketDropVisualizerBase *>(this)->removePacketDropVisualization(packetDrop);
        delete packetDrop;
    }
}

void PacketDropVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == IMobility::mobilityStateChangedSignal) {
        // TODO: update packet drop positions
    }
    else if (signal == LayeredProtocolBase::packetFromLowerDroppedSignal || signal == LayeredProtocolBase::packetFromUpperDroppedSignal) {
        if (packetNameMatcher.matches(object->getFullName()))
            addPacketDropVisualization(createPacketDropVisualization(check_and_cast<cModule*>(source), check_and_cast<cPacket*>(object)->dup()));
    }
}

void PacketDropVisualizerBase::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    packetDropVisualizations.push_back(packetDropVisualization);
}

void PacketDropVisualizerBase::removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    packetDropVisualizations.erase(std::remove(packetDropVisualizations.begin(), packetDropVisualizations.end(), packetDropVisualization), packetDropVisualizations.end());
}

} // namespace visualizer

} // namespace inet

