//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/PacketDropVisualizerBase.h"

#include <algorithm>

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

PacketDrop::PacketDrop(PacketDropReason reason, const cPacket *packet, const int moduleId, const Coord& position) :
    packet(packet),
    moduleId(moduleId),
    position(position)
{
    this->reason = reason;
}

PacketDrop::~PacketDrop()
{
    delete packet;
}

const cModule *PacketDrop::getModule() const
{
    return check_and_cast<cModule *>(cSimulation::getActiveSimulation()->getComponent(moduleId));
}

const cModule *PacketDrop::getNetworkNode() const
{
    auto module = getModule();
    return module != nullptr ? findContainingNode(module) : nullptr;
}

const NetworkInterface *PacketDrop::getNetworkInterface() const
{
    auto module = getModule();
    return module != nullptr ? findContainingNicModule(module) : nullptr;
}

PacketDropVisualizerBase::PacketDropVisualization::PacketDropVisualization(const PacketDrop *packetDrop) :
    packetDrop(packetDrop)
{
}

PacketDropVisualizerBase::PacketDropVisualization::~PacketDropVisualization()
{
    delete packetDrop;
}

PacketDropVisualizerBase::DirectiveResolver::DirectiveResolver(const PacketDrop *packetDrop) :
    packetDrop(packetDrop)
{
}

std::string PacketDropVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return packetDrop->getPacket_()->getName();
        case 'c':
            return packetDrop->getPacket_()->getClassName();
        case 'r':
            return std::to_string(packetDrop->getReason());
        case 's':
            return cEnum::find("inet::PacketDropReason")->getStringFor(packetDrop->getReason());
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void PacketDropVisualizerBase::DetailsFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, true, true, true);
}

bool PacketDropVisualizerBase::DetailsFilter::matches(const PacketDropDetails *details) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, details);
    return matchExpression.matches(&matchableObject);
}

void PacketDropVisualizerBase::preDelete(cComponent *root)
{
    if (displayPacketDrops) {
        unsubscribe();
        removeAllPacketDropVisualizations();
    }
}

void PacketDropVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayPacketDrops = par("displayPacketDrops");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setExpression(par("packetFilter").objectValue());
        detailsFilter.setPattern(par("detailsFilter"));
        icon = par("icon");
        iconTintAmount = par("iconTintAmount");
        iconTintColorSet.parseColors(par("iconTintColor"));
        labelFormat.parseFormat(par("labelFormat"));
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
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    else if (!strcmp(name, "detailsFilter"))
        detailsFilter.setPattern(par("detailsFilter"));
    else if (!strcmp(name, "labelFormat"))
        labelFormat.parseFormat(par("labelFormat"));
    removeAllPacketDropVisualizations();
}

void PacketDropVisualizerBase::refreshDisplay() const
{
    AnimationPosition currentAnimationPosition;
    std::vector<const PacketDropVisualization *> removedPacketDropVisualizations;
    for (auto packetDropVisualization : packetDropVisualizations) {
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentAnimationPosition.getSimulationTime() - packetDropVisualization->packetDropAnimationPosition.getSimulationTime()).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationPosition.getAnimationTime() - packetDropVisualization->packetDropAnimationPosition.getAnimationTime();
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentAnimationPosition.getRealTime() - packetDropVisualization->packetDropAnimationPosition.getRealTime();
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        if (delta > fadeOutTime)
            removedPacketDropVisualizations.push_back(packetDropVisualization);
        else
            setAlpha(packetDropVisualization, 1 - delta / fadeOutTime);
    }
    for (auto packetDropVisualization : removedPacketDropVisualizations) {
        const_cast<PacketDropVisualizerBase *>(this)->removePacketDropVisualization(packetDropVisualization);
        delete packetDropVisualization;
    }
}

void PacketDropVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(packetDroppedSignal, this);

}

void PacketDropVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(packetDroppedSignal, this);
}

void PacketDropVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == packetDroppedSignal) {
        auto module = check_and_cast<cModule *>(source);
        auto packet = check_and_cast<cPacket *>(object);
        auto packetDropDetails = check_and_cast<PacketDropDetails *>(details);
        auto networkNode = findContainingNode(module);
        auto networkInterface = findContainingNicModule(module);
        if ((networkNode == nullptr || nodeFilter.matches(networkNode)) && (networkInterface == nullptr || interfaceFilter.matches(networkInterface)) &&
            packetFilter.matches(packet) && detailsFilter.matches(packetDropDetails))
        {
            auto position = networkNode != nullptr ? getPosition(networkNode) : Coord::NIL;
            auto packetDrop = new PacketDrop(packetDropDetails->getReason(), packet->dup(), module->getId(), position);
            auto packetDropVisualization = createPacketDropVisualization(packetDrop);
            addPacketDropVisualization(packetDropVisualization);
        }
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
    for (auto packetDropVisualization : std::vector<const PacketDropVisualization *>(packetDropVisualizations)) {
        removePacketDropVisualization(packetDropVisualization);
        delete packetDropVisualization;
    }
}

std::string PacketDropVisualizerBase::getPacketDropVisualizationText(const PacketDrop *packetDrop) const
{
    DirectiveResolver directiveResolver(packetDrop);
    return labelFormat.formatString(&directiveResolver);
}

} // namespace visualizer

} // namespace inet

