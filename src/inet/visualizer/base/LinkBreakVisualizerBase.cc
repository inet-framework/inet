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
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/linklayer/contract/IMACFrame.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/LinkBreakVisualizerBase.h"

namespace inet {

namespace visualizer {

LinkBreakVisualizerBase::LinkBreakVisualization::LinkBreakVisualization(int transmitterModuleId, int receiverModuleId, simtime_t breakSimulationTime, double breakAnimationTime, double breakRealTime) :
    transmitterModuleId(transmitterModuleId),
    receiverModuleId(receiverModuleId),
    breakSimulationTime(breakSimulationTime),
    breakAnimationTime(breakAnimationTime),
    breakRealTime(breakRealTime)
{
}

void LinkBreakVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(IMobility::mobilityStateChangedSignal, this);
        subscriptionModule->subscribe(NF_LINK_BREAK, this);
        nodeMatcher.setPattern(par("nodeFilter"), true, true, true);
        icon = par("icon");
        iconTintAmount = par("iconTintAmount");
        if (iconTintAmount != 0)
            iconTintColor = cFigure::Color(par("iconTintColor"));
        fadeOutMode = par("fadeOutMode");
        fadeOutHalfLife = par("fadeOutHalfLife");
    }
}

void LinkBreakVisualizerBase::refreshDisplay() const
{
    auto currentSimulationTime = simTime();
    double currentAnimationTime = getSimulation()->getEnvir()->getAnimationTime();
    double currentRealTime = getRealTime();
    std::vector<const LinkBreakVisualization *> removedLinkBreakVisualizations;
    for (auto it : linkBreakVisualizations) {
        auto linkBreakVisualization = it.second;
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentSimulationTime - linkBreakVisualization->breakSimulationTime).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationTime - linkBreakVisualization->breakAnimationTime;
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentRealTime - linkBreakVisualization->breakRealTime;
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        auto alpha = std::min(1.0, std::pow(2.0, -delta / fadeOutHalfLife));
        if (alpha < 0.01)
            removedLinkBreakVisualizations.push_back(linkBreakVisualization);
        else
            setAlpha(linkBreakVisualization, alpha);
    }
    for (auto linkBreakVisualization : removedLinkBreakVisualizations) {
        const_cast<LinkBreakVisualizerBase *>(this)->removeLinkBreakVisualization(linkBreakVisualization);
        delete linkBreakVisualization;
    }
}

void LinkBreakVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == IMobility::mobilityStateChangedSignal) {
        auto mobility = dynamic_cast<IMobility *>(object);
        auto position = mobility->getCurrentPosition();
        auto module = check_and_cast<cModule *>(source);
        auto node = getContainingNode(module);
        setPosition(node, position);
    }
    else if (signal == NF_LINK_BREAK) {
        auto frame = dynamic_cast<IMACFrame *>(object);
        if (frame != nullptr) {
            auto transmitter = findNode(frame->getTransmitterAddress());
            auto receiver = findNode(frame->getReceiverAddress());
            if (nodeMatcher.matches(transmitter->getFullPath().c_str()) && nodeMatcher.matches(receiver->getFullPath().c_str())) {
                auto key = std::pair<int, int>(transmitter->getId(), receiver->getId());
                auto it = linkBreakVisualizations.find(key);
                if (it == linkBreakVisualizations.end())
                    addLinkBreakVisualization(createLinkBreakVisualization(transmitter, receiver));
                else {
                    auto linkBreakVisualization = it->second;
                    linkBreakVisualization->breakSimulationTime = simTime();
                    linkBreakVisualization->breakAnimationTime = getSimulation()->getEnvir()->getAnimationTime();
                    linkBreakVisualization->breakRealTime = getRealTime();
                }
            }
        }
    }
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

// TODO: inefficient, create L2AddressResolver?
cModule *LinkBreakVisualizerBase::findNode(MACAddress address)
{
    L3AddressResolver addressResolver;
    for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
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

} // namespace visualizer

} // namespace inet

