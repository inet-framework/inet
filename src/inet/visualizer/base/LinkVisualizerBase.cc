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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/LinkVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkVisualizerBase::LinkVisualization::LinkVisualization(int sourceModuleId, int destinationModuleId) :
    sourceModuleId(sourceModuleId),
    destinationModuleId(destinationModuleId)
{
}

void LinkVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
        subscriptionModule->subscribe(IMobility::mobilityStateChangedSignal, this);
        packetNameMatcher.setPattern(par("packetNameFilter"), false, true, true);
        lineColor = cFigure::Color(par("lineColor"));
        lineWidth = par("lineWidth");
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        fadeOutMode = par("fadeOutMode");
        fadeOutHalfLife = par("fadeOutHalfLife");
    }
}

void LinkVisualizerBase::refreshDisplay() const
{
    auto currentSimulationTime = simTime();
    double currentAnimationTime = getSimulation()->getEnvir()->getAnimationTime();
    double currentRealTime = getRealTime();
    std::vector<const LinkVisualization *> removedLinkVisualizations;
    for (auto it : linkVisualizations) {
        auto linkVisualization = it.second;
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentSimulationTime - linkVisualization->lastUsageSimulationTime).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationTime - linkVisualization->lastUsageAnimationTime;
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentRealTime - linkVisualization->lastUsageRealTime;
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        auto alpha = std::min(1.0, std::pow(2.0, -delta / fadeOutHalfLife));
        if (alpha < 0.01)
            removedLinkVisualizations.push_back(linkVisualization);
        else
            setAlpha(linkVisualization, alpha);
    }
    for (auto linkVisualization : removedLinkVisualizations) {
        const_cast<LinkVisualizerBase *>(this)->removeLinkVisualization(linkVisualization);
        delete linkVisualization;
    }
}

const LinkVisualizerBase::LinkVisualization *LinkVisualizerBase::getLinkVisualization(std::pair<int, int> linkVisualization)
{
    auto it = linkVisualizations.find(linkVisualization);
    return it == linkVisualizations.end() ? nullptr : it->second;
}

void LinkVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    linkVisualizations[sourceAndDestination] = linkVisualization;
}

void LinkVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    linkVisualizations.erase(linkVisualizations.find(std::pair<int, int>(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId)));
}

cModule *LinkVisualizerBase::getLastModule(int treeId)
{
    auto it = lastModules.find(treeId);
    if (it == lastModules.end())
        return nullptr;
    else
        return getSimulation()->getModule(it->second);
}

void LinkVisualizerBase::setLastModule(int treeId, cModule *module)
{
    lastModules[treeId] = module->getId();
}

void LinkVisualizerBase::removeLastModule(int treeId)
{
    lastModules.erase(lastModules.find(treeId));
}

void LinkVisualizerBase::updateLinkVisualization(cModule *source, cModule *destination)
{
    auto key = std::pair<int, int>(source->getId(), destination->getId());
    auto linkVisualization = getLinkVisualization(key);
    if (linkVisualization == nullptr) {
        linkVisualization = createLinkVisualization(source, destination);
        addLinkVisualization(key, linkVisualization);
    }
    else {
        linkVisualization->lastUsageSimulationTime = getSimulation()->getSimTime();
        linkVisualization->lastUsageAnimationTime = getSimulation()->getEnvir()->getAnimationTime();
        linkVisualization->lastUsageRealTime = getRealTime();
    }
}

void LinkVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == IMobility::mobilityStateChangedSignal) {
        auto mobility = dynamic_cast<IMobility *>(object);
        auto position = mobility->getCurrentPosition();
        auto module = check_and_cast<cModule *>(source);
        auto node = getContainingNode(module);
        setPosition(node, position);
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromUpperSignal) {
        if (isLinkEnd(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                setLastModule(treeId, module);
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetSentToUpperSignal) {
        if (isLinkEnd(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                auto lastModule = getLastModule(treeId);
                if (lastModule != nullptr) {
                    updateLinkVisualization(getContainingNode(lastModule), getContainingNode(module));
                    // TODO: breaks due to multiple recipient?
                    // removeLastModule(treeId);
                }
            }
        }
    }
}

} // namespace visualizer

} // namespace inet

