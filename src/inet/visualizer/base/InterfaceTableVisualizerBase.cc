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
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"

namespace inet {

namespace visualizer {

InterfaceTableVisualizerBase::InterfaceVisualization::InterfaceVisualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

InterfaceTableVisualizerBase::~InterfaceTableVisualizerBase()
{
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(NF_INTERFACE_CREATED, this);
        subscriptionModule->unsubscribe(NF_INTERFACE_DELETED, this);
        subscriptionModule->unsubscribe(NF_INTERFACE_CONFIG_CHANGED, this);
        subscriptionModule->unsubscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
}

void InterfaceTableVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(NF_INTERFACE_CREATED, this);
        subscriptionModule->subscribe(NF_INTERFACE_DELETED, this);
        subscriptionModule->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
        subscriptionModule->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
        nodeMatcher.setPattern(par("nodeFilter"), true, true, true);
        interfaceMatcher.setPattern(par("interfaceFilter"), false, true, true);
        content = par("content");
        fontColor = cFigure::parseColor(par("fontColor"));
        backgroundColor = cFigure::parseColor(par("backgroundColor"));
        opacity = par("opacity");
    }
}

const InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableVisualizerBase::getInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry)
{
    auto key = std::pair<int, int>(networkNode->getId(), interfaceEntry->getInterfaceId());
    auto it = interfaceVisualizations.find(key);
    return it == interfaceVisualizations.end() ? nullptr : it->second;
}

void InterfaceTableVisualizerBase::addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    auto key = std::pair<int, int>(interfaceVisualization->networkNodeId, interfaceVisualization->interfaceId);
    interfaceVisualizations[key] = interfaceVisualization;
}

void InterfaceTableVisualizerBase::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    auto key = std::pair<int, int>(interfaceVisualization->networkNodeId, interfaceVisualization->interfaceId);
    interfaceVisualizations.erase(interfaceVisualizations.find(key));
}

std::string InterfaceTableVisualizerBase::getVisualizationText(const InterfaceEntry *interfaceEntry)
{
    if (!strcmp(content, "networkAddress"))
        return interfaceEntry->getNetworkAddress().str();
    else if (!strcmp(content, "macAddress"))
        return interfaceEntry->getMacAddress().str();
    else if (!strcmp(content, "info"))
        return interfaceEntry->info();
    else
        throw cRuntimeError("Unknown content parameter");
}

void InterfaceTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == NF_INTERFACE_CREATED) {
        if (nodeMatcher.matches(source->getFullPath().c_str())) {
            auto interfaceEntry = static_cast<InterfaceEntry *>(object);
            if (interfaceMatcher.matches(interfaceEntry->getFullName())) {
                auto networkNode = getContainingNode(static_cast<cModule *>(source));
                auto interfaceVisualization = createInterfaceVisualization(networkNode, interfaceEntry);
                addInterfaceVisualization(interfaceVisualization);
            }
        }
    }
    if (signal == NF_INTERFACE_DELETED) {
        if (nodeMatcher.matches(source->getFullPath().c_str())) {
            auto interfaceEntry = static_cast<InterfaceEntry *>(object);
            if (interfaceMatcher.matches(interfaceEntry->getFullName())) {
                auto networkNode = getContainingNode(static_cast<cModule *>(source));
                auto interfaceVisualization = getInterfaceVisualization(networkNode, interfaceEntry);
                removeInterfaceVisualization(interfaceVisualization);
            }
        }
    }
    else if (signal == NF_INTERFACE_CONFIG_CHANGED || signal == NF_INTERFACE_IPv4CONFIG_CHANGED) {
        if (object != nullptr && nodeMatcher.matches(source->getFullPath().c_str())) {
            auto interfaceEntryDetails = static_cast<InterfaceEntryChangeDetails *>(object);
            auto interfaceEntry = interfaceEntryDetails->getInterfaceEntry();
            auto fieldId = interfaceEntryDetails->getFieldId();
            if (fieldId == InterfaceEntry::F_IPV4_DATA || fieldId == IPv4InterfaceData::F_IP_ADDRESS) {
                if (interfaceMatcher.matches(interfaceEntry->getFullName())) {
                    auto networkNode = getContainingNode(static_cast<cModule *>(source));
                    auto interfaceVisualization = getInterfaceVisualization(networkNode, interfaceEntry);
                    refreshInterfaceVisualization(interfaceVisualization, interfaceEntry);
                }
            }
        }
    }
}

} // namespace visualizer

} // namespace inet

