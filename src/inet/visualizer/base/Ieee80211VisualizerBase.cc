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
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"

namespace inet {

namespace visualizer {

Ieee80211VisualizerBase::Ieee80211Visualization::Ieee80211Visualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

void Ieee80211VisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(NF_L2_ASSOCIATED, this);
        subscriptionModule->subscribe(NF_L2_DISASSOCIATED, this);
        subscriptionModule->subscribe(NF_L2_AP_ASSOCIATED, this);
        subscriptionModule->subscribe(NF_L2_AP_DISASSOCIATED, this);
        nodeMatcher.setPattern(par("nodeFilter"), true, true, true);
        interfaceMatcher.setPattern(par("interfaceFilter"), false, true, true);
    }
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211VisualizerBase::getIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry)
{
    auto key = std::pair<int, int>(networkNode->getId(), interfaceEntry->getInterfaceId());
    auto it = ieee80211Visualizations.find(key);
    return it == ieee80211Visualizations.end() ? nullptr : it->second;
}

void Ieee80211VisualizerBase::addIeee80211Visualization(Ieee80211Visualization *ieee80211Visualization)
{
    auto key = std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId);
    ieee80211Visualizations[key] = ieee80211Visualization;
}

void Ieee80211VisualizerBase::removeIeee80211Visualization(Ieee80211Visualization *ieee80211Visualization)
{
    ieee80211Visualizations.erase(ieee80211Visualizations.find(std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId)));
}


void Ieee80211VisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == NF_L2_ASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeMatcher.matches(networkNode->getFullPath().c_str())) {
            auto interfaceEntry = check_and_cast<InterfaceEntry *>(object);
            auto ieee80211Visualization = createIeee80211Visualization(networkNode, interfaceEntry);
            addIeee80211Visualization(ieee80211Visualization);
        }
    }
    else if (signal == NF_L2_DISASSOCIATED) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeMatcher.matches(networkNode->getFullPath().c_str())) {
            auto interfaceEntry = check_and_cast<InterfaceEntry *>(object);
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, interfaceEntry);
            removeIeee80211Visualization(ieee80211Visualization);
        }
    }
}

} // namespace visualizer

} // namespace inet

