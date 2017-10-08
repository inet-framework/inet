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

#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.h"
#endif // WITH_IEEE80211
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"

namespace inet {

namespace visualizer {

Ieee80211VisualizerBase::Ieee80211Visualization::Ieee80211Visualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

Ieee80211VisualizerBase::~Ieee80211VisualizerBase()
{
    if (displayAssociations)
        unsubscribe();
}

void Ieee80211VisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayAssociations = par("displayAssociations");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        icon = par("icon");
        iconColorSet.parseColors(par("iconColor"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        if (displayAssociations)
            subscribe();
    }
}

void Ieee80211VisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        removeAllIeee80211Visualizations();
    }
}

void Ieee80211VisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(NF_L2_ASSOCIATED, this);
    subscriptionModule->subscribe(NF_L2_DISASSOCIATED, this);
    subscriptionModule->subscribe(NF_L2_AP_ASSOCIATED, this);
    subscriptionModule->subscribe(NF_L2_AP_DISASSOCIATED, this);
}

void Ieee80211VisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(NF_L2_ASSOCIATED, this);
        subscriptionModule->unsubscribe(NF_L2_DISASSOCIATED, this);
        subscriptionModule->unsubscribe(NF_L2_AP_ASSOCIATED, this);
        subscriptionModule->unsubscribe(NF_L2_AP_DISASSOCIATED, this);
    }
}

const Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211VisualizerBase::getIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry)
{
    auto key = std::pair<int, int>(networkNode->getId(), interfaceEntry->getInterfaceId());
    auto it = ieee80211Visualizations.find(key);
    return it == ieee80211Visualizations.end() ? nullptr : it->second;
}

void Ieee80211VisualizerBase::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    auto key = std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId);
    ieee80211Visualizations[key] = ieee80211Visualization;
}

void Ieee80211VisualizerBase::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    auto key = std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId);
    ieee80211Visualizations.erase(ieee80211Visualizations.find(key));
}

void Ieee80211VisualizerBase::removeAllIeee80211Visualizations()
{
    std::vector<const Ieee80211Visualization *> removedIeee80211Visualizations;
    for (auto it : ieee80211Visualizations)
        removedIeee80211Visualizations.push_back(it.second);
    for (auto it : removedIeee80211Visualizations) {
        removeIeee80211Visualization(it);
        delete it;
    }
}

void Ieee80211VisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
#ifdef WITH_IEEE80211
    Enter_Method_Silent();
    if (signal == NF_L2_ASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto interfaceEntry = check_and_cast<InterfaceEntry *>(object);
            auto apInfo = check_and_cast<inet::ieee80211::Ieee80211MgmtSTA::APInfo *>(details);
            auto ieee80211Visualization = createIeee80211Visualization(networkNode, interfaceEntry, apInfo->ssid);
            addIeee80211Visualization(ieee80211Visualization);
        }
    }
    else if (signal == NF_L2_DISASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto interfaceEntry = check_and_cast<InterfaceEntry *>(object);
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, interfaceEntry);
            removeIeee80211Visualization(ieee80211Visualization);
        }
    }
    else if (signal == NF_L2_AP_ASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            // TODO: KLUDGE: this is the wrong way to lookup the interface and the ssid
            L3AddressResolver addressResolver;
            auto mgmt = check_and_cast<inet::ieee80211::Ieee80211MgmtAP *>(source);
            auto interfaceEntry = addressResolver.findInterfaceTableOf(networkNode)->getInterfaceByInterfaceModule(mgmt->getParentModule());
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, interfaceEntry);
            if (ieee80211Visualization == nullptr) {
                auto ieee80211Visualization = createIeee80211Visualization(networkNode, interfaceEntry, mgmt->par("ssid"));
                addIeee80211Visualization(ieee80211Visualization);
            }
        }
    }
    else if (signal == NF_L2_AP_DISASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            // TODO: KLUDGE: this is the wrong way to lookup the interface
            L3AddressResolver addressResolver;
            auto mgmt = check_and_cast<inet::ieee80211::Ieee80211MgmtAP *>(source);
            auto interfaceEntry = addressResolver.findInterfaceTableOf(networkNode)->getInterfaceByInterfaceModule(mgmt->getParentModule());
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, interfaceEntry);
            removeIeee80211Visualization(ieee80211Visualization);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
#endif // WITH_IEEE80211
}

} // namespace visualizer

} // namespace inet

