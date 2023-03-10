//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif // INET_WITH_IEEE80211
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/LinkBreakVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkBreakVisualizerBase::LinkBreakVisualization::LinkBreakVisualization(int transmitterModuleId, int receiverModuleId) :
    transmitterModuleId(transmitterModuleId),
    receiverModuleId(receiverModuleId)
{
}

void LinkBreakVisualizerBase::preDelete(cComponent *root)
{
    if (displayLinkBreaks) {
        unsubscribe();
        removeAllLinkBreakVisualizations();
    }
}

void LinkBreakVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayLinkBreaks = par("displayLinkBreaks");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setExpression(par("packetFilter").objectValue());
        icon = par("icon");
        iconTintAmount = par("iconTintAmount");
        if (iconTintAmount != 0)
            iconTintColor = cFigure::Color(par("iconTintColor"));
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        if (displayLinkBreaks)
            subscribe();
    }
}

void LinkBreakVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    removeAllLinkBreakVisualizations();
}

void LinkBreakVisualizerBase::refreshDisplay() const
{
    AnimationPosition currentAnimationPosition;
    std::vector<const LinkBreakVisualization *> removedLinkBreakVisualizations;
    for (auto it : linkBreakVisualizations) {
        auto linkBreakVisualization = it.second;
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentAnimationPosition.getSimulationTime() - linkBreakVisualization->linkBreakAnimationPosition.getSimulationTime()).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationPosition.getAnimationTime() - linkBreakVisualization->linkBreakAnimationPosition.getAnimationTime();
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentAnimationPosition.getRealTime() - linkBreakVisualization->linkBreakAnimationPosition.getRealTime();
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        if (delta > fadeOutTime)
            removedLinkBreakVisualizations.push_back(linkBreakVisualization);
        else
            setAlpha(linkBreakVisualization, 1 - delta / fadeOutTime);
    }
    for (auto linkBreakVisualization : removedLinkBreakVisualizations) {
        const_cast<LinkBreakVisualizerBase *>(this)->removeLinkBreakVisualization(linkBreakVisualization);
        delete linkBreakVisualization;
    }
}

void LinkBreakVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(linkBrokenSignal, this);
}

void LinkBreakVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(linkBrokenSignal, this);
}

void LinkBreakVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == linkBrokenSignal) {
        MacAddress transmitterAddress;
        MacAddress receiverAddress;
        // TODO revive
//        if (auto frame = dynamic_cast<IMACFrame *>(object)) {
//            transmitterAddress = frame->getTransmitterAddress();
//            receiverAddress = frame->getReceiverAddress();
//        }
#ifdef INET_WITH_IEEE80211
        if (auto frame = dynamic_cast<ieee80211::Ieee80211TwoAddressHeader *>(object)) {
            transmitterAddress = frame->getTransmitterAddress();
            receiverAddress = frame->getReceiverAddress();
        }
#endif // INET_WITH_IEEE80211
        auto transmitter = findNode(transmitterAddress);
        auto receiver = findNode(receiverAddress);
        if (nodeFilter.matches(transmitter) && nodeFilter.matches(receiver)) {
            auto key = std::pair<int, int>(transmitter->getId(), receiver->getId());
            auto it = linkBreakVisualizations.find(key);
            if (it == linkBreakVisualizations.end())
                addLinkBreakVisualization(createLinkBreakVisualization(transmitter, receiver));
            else {
                auto linkBreakVisualization = it->second;
                linkBreakVisualization->linkBreakAnimationPosition = AnimationPosition();
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

void LinkBreakVisualizerBase::addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    auto key = std::pair<int, int>(linkBreakVisualization->transmitterModuleId, linkBreakVisualization->receiverModuleId);
    linkBreakVisualizations[key] = linkBreakVisualization;
}

void LinkBreakVisualizerBase::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    auto key = std::pair<int, int>(linkBreakVisualization->transmitterModuleId, linkBreakVisualization->receiverModuleId);
    auto it = linkBreakVisualizations.find(key);
    linkBreakVisualizations.erase(it);
}

// TODO inefficient, create L2AddressResolver?
cModule *LinkBreakVisualizerBase::findNode(MacAddress address)
{
    L3AddressResolver addressResolver;
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode)) {
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
                if (interfaceTable->getInterface(i)->getMacAddress() == address)
                    return networkNode;
        }
    }
    return nullptr;
}

void LinkBreakVisualizerBase::removeAllLinkBreakVisualizations()
{
    std::vector<const LinkBreakVisualization *> removedLinkBreakVisualizations;
    for (auto it : linkBreakVisualizations)
        removedLinkBreakVisualizations.push_back(it.second);
    for (auto it : removedLinkBreakVisualizations) {
        removeLinkBreakVisualization(it);
        delete it;
    }
}

} // namespace visualizer

} // namespace inet

