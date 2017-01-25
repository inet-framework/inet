//
// Copyright (C) 2016 OpenSim Ltd.
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

LinkVisualizerBase::Link::Link(int sourceModuleId, int destinationModuleId) :
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
        opacityHalfLife = par("opacityHalfLife");
    }
}

void LinkVisualizerBase::refreshDisplay() const
{
    auto now = simTime();
    std::vector<const Link *> removedLinks;
    for (auto it : links) {
        auto link = it.second;
        auto alpha = std::min(1.0, std::pow(2.0, -(now - link->lastUsage).dbl() / opacityHalfLife));
        if (alpha < 0.01)
            removedLinks.push_back(link);
        else
            setAlpha(link, alpha);
    }
    for (auto link : removedLinks) {
        const_cast<LinkVisualizerBase *>(this)->removeLink(link);
        delete link;
    }
}

const LinkVisualizerBase::Link *LinkVisualizerBase::getLink(std::pair<int, int> link)
{
    auto it = links.find(link);
    return it == links.end() ? nullptr : it->second;
}

void LinkVisualizerBase::addLink(std::pair<int, int> sourceAndDestination, const Link *link)
{
    links[sourceAndDestination] = link;
}

void LinkVisualizerBase::removeLink(const Link *link)
{
    links.erase(links.find(std::pair<int, int>(link->sourceModuleId, link->destinationModuleId)));
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

void LinkVisualizerBase::updateLink(cModule *source, cModule *destination)
{
    auto key = std::pair<int, int>(source->getId(), destination->getId());
    auto link = getLink(key);
    if (link == nullptr) {
        link = createLink(source, destination);
        addLink(key, link);
    }
    else
        link->lastUsage = simTime();
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
                    updateLink(getContainingNode(lastModule), getContainingNode(module));
                    // TODO: breaks due to multiple recipient?
                    // removeLastModule(treeId);
                }
            }
        }
    }
}

} // namespace visualizer

} // namespace inet

