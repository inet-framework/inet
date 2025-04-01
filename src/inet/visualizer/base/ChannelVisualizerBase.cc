//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/ChannelVisualizerBase.h"

namespace inet {

namespace visualizer {

static simsignal_t messageSentSignal = cComponent::registerSignal("messageSent");

ChannelVisualizerBase::ChannelVisualization::ChannelVisualization(int sourceModuleId, int destinationModuleId) :
    LineManager::ModuleLine(sourceModuleId, destinationModuleId)
{
}

std::string ChannelVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
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

void ChannelVisualizerBase::preDelete(cComponent *root)
{
    if (displayChannelActivity) {
        unsubscribe();
        removeAllChannelVisualizations();
    }
}

void ChannelVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayChannelActivity = par("displayChannelActivity");
        nodeFilter.setPattern(par("nodeFilter"));
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
        if (displayChannelActivity)
            subscribe();
    }
}

void ChannelVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    removeAllChannelVisualizations();
}

void ChannelVisualizerBase::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    if (displayChannelActivity) {
        AnimationPosition currentAnimationPosition;
        std::vector<const ChannelVisualization *> removedChannelVisualizations;
        for (auto it : channelVisualizations) {
            auto channelVisualization = it.second;
            double delta;
            if (!strcmp(fadeOutMode, "simulationTime"))
                delta = (currentAnimationPosition.getSimulationTime() - channelVisualization->lastUsageAnimationPosition.getSimulationTime()).dbl();
            else if (!strcmp(fadeOutMode, "animationTime"))
                delta = currentAnimationPosition.getAnimationTime() - channelVisualization->lastUsageAnimationPosition.getAnimationTime();
            else if (!strcmp(fadeOutMode, "realTime"))
                delta = currentAnimationPosition.getRealTime() - channelVisualization->lastUsageAnimationPosition.getRealTime();
            else
                throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
            if (delta > fadeOutTime)
                removedChannelVisualizations.push_back(channelVisualization);
            else
                setAlpha(channelVisualization, 1 - delta / fadeOutTime);
        }
        for (auto channelVisualization : removedChannelVisualizations) {
            const_cast<ChannelVisualizerBase *>(this)->removeChannelVisualization(channelVisualization);
            delete channelVisualization;
        }
    }
}

void ChannelVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(messageSentSignal, this);
}

void ChannelVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    visualizationSubjectModule->unsubscribe(messageSentSignal, this);
}

const ChannelVisualizerBase::ChannelVisualization *ChannelVisualizerBase::getChannelVisualization(std::pair<int, int> channelVisualization)
{
    auto it = channelVisualizations.find(channelVisualization);
    return it == channelVisualizations.end() ? nullptr : it->second;
}

void ChannelVisualizerBase::addChannelVisualization(std::pair<int, int> sourceAndDestination, const ChannelVisualization *channelVisualization)
{
    channelVisualizations[sourceAndDestination] = channelVisualization;
    if (holdAnimationTime != 0)
        visualizationTargetModule->getCanvas()->holdSimulationFor(holdAnimationTime);
}

void ChannelVisualizerBase::removeChannelVisualization(const ChannelVisualization *channelVisualization)
{
    channelVisualizations.erase(channelVisualizations.find(std::pair<int, int>(channelVisualization->sourceModuleId, channelVisualization->destinationModuleId)));
}

void ChannelVisualizerBase::removeAllChannelVisualizations()
{
    std::vector<const ChannelVisualization *> removedChannelVisualizations;
    for (auto it : channelVisualizations)
        removedChannelVisualizations.push_back(it.second);
    for (auto it : removedChannelVisualizations) {
        removeChannelVisualization(it);
        delete it;
    }
}

std::string ChannelVisualizerBase::getChannelVisualizationText(cPacket *packet) const
{
    DirectiveResolver directiveResolver(packet);
    return labelFormat.formatString(&directiveResolver);
}

void ChannelVisualizerBase::refreshChannelVisualization(const ChannelVisualization *channelVisualization, cPacket *packet)
{
    channelVisualization->lastUsageAnimationPosition = AnimationPosition();
}

void ChannelVisualizerBase::updateChannelVisualization(cModule *source, cModule *destination, cPacket *packet)
{
    auto key = std::pair<int, int>(source->getId(), destination->getId());
    auto channelVisualization = getChannelVisualization(key);
    if (channelVisualization == nullptr) {
        channelVisualization = createChannelVisualization(source, destination, packet);
        addChannelVisualization(key, channelVisualization);
    }
    else
        refreshChannelVisualization(channelVisualization, packet);
}

void ChannelVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == messageSentSignal) {
        auto messageSentSignalValue = check_and_cast<cChannel::MessageSentSignalValue *>(object);
        auto signal = check_and_cast<cPacket *>(messageSentSignalValue->msg);
        auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
        auto channel = check_and_cast<cChannel *>(source);
        cModule *sourceNode = getContainingNode(channel->getSourceGate()->getOwnerModule());
        cModule *destinationNode = getContainingNode(channel->getSourceGate()->getPathEndGate()->getOwnerModule());
        if (nodeFilter.matches(sourceNode) && nodeFilter.matches(destinationNode) && packetFilter.matches(packet))
            updateChannelVisualization(sourceNode, destinationNode, packet);
    }
}

} // namespace visualizer

} // namespace inet

