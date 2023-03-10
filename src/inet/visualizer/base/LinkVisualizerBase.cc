//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/LinkVisualizerBase.h"

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

LinkVisualizerBase::LinkVisualization::LinkVisualization(int sourceModuleId, int destinationModuleId) :
    LineManager::ModuleLine(sourceModuleId, destinationModuleId)
{
}

std::string LinkVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return packet->getName();
        case 'c':
            return packet->getClassName();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void LinkVisualizerBase::preDelete(cComponent *root)
{
    if (displayLinks) {
        unsubscribe();
        removeAllLinkVisualizations();
    }
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
        packetFilter.setExpression(par("packetFilter").objectValue());
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
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    removeAllLinkVisualizations();
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
    return (it == lastModules.end()) ? nullptr : getSimulation()->getModule(it->second);
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
    Enter_Method("%s", cComponent::getSignalName(signal));

    if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetReceivedFromUpperSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetSentToPeerSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetSentToLowerSignal))
    {
        if (isLinkStart(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto packet = check_and_cast<Packet *>(object);
            mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) { if (getLastModule(id) != nullptr) removeLastModule(id); });
            auto networkNode = getContainingNode(module);
            auto networkInterface = getContainingNicModule(module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(networkInterface) && packetFilter.matches(packet)) {
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
            auto networkInterface = getContainingNicModule(module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(networkInterface) && packetFilter.matches(packet)) {
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

