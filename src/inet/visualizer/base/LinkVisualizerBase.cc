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
#include "inet/common/packet/Packet.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/LinkVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkVisualizerBase::LinkVisualization::LinkVisualization(int sourceModuleId, int destinationModuleId) :
    LineManager::ModuleLine(sourceModuleId, destinationModuleId)
{
}

const char *LinkVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'n':
            result = packet->getName();
            break;
        case 'c':
            result = packet->getClassName();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

LinkVisualizerBase::~LinkVisualizerBase()
{
    if (displayLinks)
        unsubscribe();
}

void LinkVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayLinks = par("displayLinks");
        const char *activityLevelString = par("activityLevel");
        if (!strcmp(activityLevelString, "service"))
            activityLevel = ACTIVITY_LEVEL_SERVICE;
        else if (!strcmp(activityLevelString, "peer"))
            activityLevel = ACTIVITY_LEVEL_PEER;
        else if (!strcmp(activityLevelString, "protocol"))
            activityLevel = ACTIVITY_LEVEL_PROTOCOL;
        else
            throw cRuntimeError("Unknown activity level: %s", activityLevelString);
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        lineColor = cFigure::Color(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
        lineShift = par("lineShift");
        lineShiftMode = par("lineShiftMode");
        lineContactSpacing = par("lineContactSpacing");
        lineContactMode = par("lineContactMode");
        labelFormat.parseFormat(par("labelFormat"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        holdAnimationTime = par("holdAnimationTime");
        if (displayLinks)
            subscribe();
    }
}

void LinkVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "packetFilter") || !strcmp(name, "packetDataFilter"))
            packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        removeAllLinkVisualizations();
    }
}

void LinkVisualizerBase::refreshDisplay() const
{
    if (displayLinks) {
        AnimationPosition currentAnimationPosition;
        std::vector<const LinkVisualization *> removedLinkVisualizations;
        for (auto it : linkVisualizations) {
            auto linkVisualization = it.second;
            double delta;
            if (!strcmp(fadeOutMode, "simulationTime"))
                delta = (currentAnimationPosition.getSimulationTime() - linkVisualization->lastUsageAnimationPosition.getSimulationTime()).dbl();
            else if (!strcmp(fadeOutMode, "animationTime"))
                delta = currentAnimationPosition.getAnimationTime() - linkVisualization->lastUsageAnimationPosition.getAnimationTime();
            else if (!strcmp(fadeOutMode, "realTime"))
                delta = currentAnimationPosition.getRealTime() - linkVisualization->lastUsageAnimationPosition.getRealTime();
            else
                throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
            if (delta > fadeOutTime)
                removedLinkVisualizations.push_back(linkVisualization);
            else
                setAlpha(linkVisualization, 1 - delta / fadeOutTime);
        }
        for (auto linkVisualization : removedLinkVisualizations) {
            const_cast<LinkVisualizerBase *>(this)->removeLinkVisualization(linkVisualization);
            delete linkVisualization;
        }
    }
}

void LinkVisualizerBase::subscribe()
{
    if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
        visualizationSubjectModule->subscribe(packetSentToUpperSignal, this);
        visualizationSubjectModule->subscribe(packetReceivedFromUpperSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PEER) {
        visualizationSubjectModule->subscribe(packetSentToPeerSignal, this);
        visualizationSubjectModule->subscribe(packetReceivedFromPeerSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
        visualizationSubjectModule->subscribe(packetSentToLowerSignal, this);
        visualizationSubjectModule->subscribe(packetReceivedFromLowerSignal, this);
    }
}

void LinkVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
            visualizationSubjectModule->unsubscribe(packetSentToUpperSignal, this);
            visualizationSubjectModule->unsubscribe(packetReceivedFromUpperSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PEER) {
            visualizationSubjectModule->unsubscribe(packetSentToPeerSignal, this);
            visualizationSubjectModule->unsubscribe(packetReceivedFromPeerSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
            visualizationSubjectModule->unsubscribe(packetSentToLowerSignal, this);
            visualizationSubjectModule->unsubscribe(packetReceivedFromLowerSignal, this);
        }
    }
}

std::string LinkVisualizerBase::getLinkVisualizationText(cPacket *packet) const
{
    DirectiveResolver directiveResolver(packet);
    return labelFormat.formatString(&directiveResolver);
}

const LinkVisualizerBase::LinkVisualization *LinkVisualizerBase::getLinkVisualization(std::pair<int, int> linkVisualization)
{
    auto it = linkVisualizations.find(linkVisualization);
    return it == linkVisualizations.end() ? nullptr : it->second;
}

void LinkVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    linkVisualizations[sourceAndDestination] = linkVisualization;
    if (holdAnimationTime != 0)
        visualizationTargetModule->getCanvas()->holdSimulationFor(holdAnimationTime);
}

void LinkVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    linkVisualizations.erase(linkVisualizations.find(std::pair<int, int>(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId)));
}

void LinkVisualizerBase::removeAllLinkVisualizations()
{
    lastModules.clear();
    std::vector<const LinkVisualization *> removedLinkVisualizations;
    for (auto it : linkVisualizations)
        removedLinkVisualizations.push_back(it.second);
    for (auto it : removedLinkVisualizations) {
        removeLinkVisualization(it);
        delete it;
    }
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

void LinkVisualizerBase::refreshLinkVisualization(const LinkVisualization *linkVisualization, cPacket *packet)
{
    linkVisualization->lastUsageAnimationPosition = AnimationPosition();
}

void LinkVisualizerBase::updateLinkVisualization(cModule *source, cModule *destination, cPacket *packet)
{
    auto key = std::pair<int, int>(source->getId(), destination->getId());
    auto linkVisualization = getLinkVisualization(key);
    if (linkVisualization == nullptr) {
        linkVisualization = createLinkVisualization(source, destination, packet);
        addLinkVisualization(key, linkVisualization);
    }
    else
        refreshLinkVisualization(linkVisualization, packet);
}

void LinkVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetReceivedFromUpperSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetSentToPeerSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetSentToLowerSignal))
    {
        if (isLinkStart(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto packet = check_and_cast<Packet *>(object);
            mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) { if (getLastModule(id) != nullptr) removeLastModule(id); });
            auto networkNode = getContainingNode(module);
            auto interfaceEntry = getContainingNicModule(module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(interfaceEntry) && packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) { setLastModule(id, module); });
            }
        }
    }
    else if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetSentToUpperSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetReceivedFromPeerSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetReceivedFromLowerSignal))
    {
        if (isLinkEnd(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto packet = check_and_cast<Packet *>(object);
            auto networkNode = getContainingNode(module);
            auto interfaceEntry = getContainingNicModule(module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(interfaceEntry) && packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) {
                    auto lastModule = getLastModule(id);
                    if (lastModule != nullptr)
                        updateLinkVisualization(getContainingNode(lastModule), networkNode, packet);
                    // NOTE: don't call removeLastModule(treeId) because other network nodes may still receive this packet
                });
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

