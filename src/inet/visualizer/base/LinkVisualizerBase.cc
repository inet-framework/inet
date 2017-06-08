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
    LineManager::ModuleLine(sourceModuleId, destinationModuleId)
{
}

const char *LinkVisualizerBase::DirectiveResolver::resolveDirective(char directive)
{
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
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setPattern(par("packetFilter"));
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
        lineManager = LineManager::getLineManager(visualizerTargetModule->getCanvas());
        if (displayLinks)
            subscribe();
    }
}

void LinkVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
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
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
}

void LinkVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
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
    if (signal == LayeredProtocolBase::packetReceivedFromUpperSignal) {
        if (isLinkStart(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto packet = check_and_cast<cPacket *>(object);
            auto treeId = packet->getTreeId();
            auto lastModule = getLastModule(treeId);
            if (lastModule != nullptr)
                removeLastModule(treeId);
            auto networkNode = getContainingNode(module);
            auto interfaceEntry = getInterfaceEntry(networkNode, module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(interfaceEntry) && packetFilter.matches(packet))
                setLastModule(treeId, module);
        }
    }
    else if (signal == LayeredProtocolBase::packetSentToUpperSignal) {
        if (isLinkEnd(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto packet = check_and_cast<cPacket *>(object);
            auto treeId = packet->getTreeId();
            auto lastModule = getLastModule(treeId);
            if (lastModule != nullptr) {
                auto networkNode = getContainingNode(module);
                auto interfaceEntry = getInterfaceEntry(networkNode, module);
                if (nodeFilter.matches(networkNode) && interfaceFilter.matches(interfaceEntry) && packetFilter.matches(packet))
                    updateLinkVisualization(getContainingNode(lastModule), networkNode, packet);
                // NOTE: don't call removeLastModule(treeId) because other network nodes may still receive this packet
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

