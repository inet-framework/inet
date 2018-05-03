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
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#ifdef WITH_NEXTHOP
#include "../../networklayer/nexthop/NextHopInterfaceData.h"
#endif // WITH_NEXTHOP
#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // WITH_IPv4
#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // WITH_IPv6
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"

namespace inet {

namespace visualizer {

InterfaceTableVisualizerBase::InterfaceVisualization::InterfaceVisualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

const char *InterfaceTableVisualizerBase::DirectiveResolver::resolveDirective(char directive)
{
    result = "";
    switch (directive) {
        case 'N':
            result = interfaceEntry->getInterfaceName();
            break;
        case 'm':
            result = interfaceEntry->getMacAddress().str();
            break;
        case 'l': // TODO: Ipv4 or Ipv6
#ifdef WITH_IPv4
            if (interfaceEntry->ipv4Data() != nullptr)
                result = std::to_string(interfaceEntry->ipv4Data()->getNetmask().getNetmaskLength());
#endif // WITH_IPv4
            break;
        case '4':
#ifdef WITH_IPv4
            if (interfaceEntry->ipv4Data() != nullptr)
                result = interfaceEntry->ipv4Data()->getIPAddress().str();
#endif // WITH_IPv4
            break;
        case '6':
#ifdef WITH_IPv6
            if (interfaceEntry->ipv6Data() != nullptr)
                result = interfaceEntry->ipv6Data()->getLinkLocalAddress().str();
#endif // WITH_IPv6
            break;
        case 'a':
            if (false) {}
#ifdef WITH_IPv4
            else if (interfaceEntry->ipv4Data() != nullptr)
                result = interfaceEntry->ipv4Data()->getIPAddress().str();
#endif // WITH_IPv4
#ifdef WITH_IPv6
            else if (interfaceEntry->ipv6Data() != nullptr)
                result = interfaceEntry->ipv6Data()->getLinkLocalAddress().str();
#endif // WITH_IPv6
#ifdef WITH_NEXTHOP
            else if (interfaceEntry->getNextHopData() != nullptr)
                result = interfaceEntry->getNextHopData()->getAddress().str();
#endif // WITH_NEXTHOP
            break;
        case 'g':
#ifdef WITH_NEXTHOP
            if (interfaceEntry->getNextHopData() != nullptr)
                result = interfaceEntry->getNextHopData()->getAddress().str();
#endif // WITH_NEXTHOP
            break;
        case 'n':
            result = interfaceEntry->getNetworkAddress().str();
            break;
        case 'i':
            result = interfaceEntry->str();
            break;
        case 's':
            result = interfaceEntry->str();
            break;
        case '\\':
            result = interfaceEntry->getNodeOutputGateId() == -1 ? "" : "\n";
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

InterfaceTableVisualizerBase::~InterfaceTableVisualizerBase()
{
    if (displayInterfaceTables)
        unsubscribe();
}

void InterfaceTableVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayInterfaceTables = par("displayInterfaceTables");
        displayWiredInterfacesAtConnections = par("displayWiredInterfacesAtConnections");
        displayBackground = par("displayBackground");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        format.parseFormat(par("format"));
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        font = cFigure::parseFont(par("font"));
        textColor = cFigure::parseColor(par("textColor"));
        backgroundColor = cFigure::parseColor(par("backgroundColor"));
        opacity = par("opacity");
        if (displayInterfaceTables)
            subscribe();
    }
}

void InterfaceTableVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "format"))
            format.parseFormat(par("format"));
        updateAllInterfaceVisualizations();
    }
}

void InterfaceTableVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(interfaceCreatedSignal, this);
    subscriptionModule->subscribe(interfaceDeletedSignal, this);
    subscriptionModule->subscribe(interfaceConfigChangedSignal, this);
    subscriptionModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
}

void InterfaceTableVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(interfaceCreatedSignal, this);
        subscriptionModule->unsubscribe(interfaceDeletedSignal, this);
        subscriptionModule->unsubscribe(interfaceConfigChangedSignal, this);
        subscriptionModule->unsubscribe(interfaceIpv4ConfigChangedSignal, this);
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

void InterfaceTableVisualizerBase::addAllInterfaceVisualizations()
{
    for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
            L3AddressResolver addressResolver;
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                    auto interfaceEntry = interfaceTable->getInterface(i);
                    if (interfaceEntry != nullptr && interfaceFilter.matches(interfaceEntry)) {
                        auto interfaceVisualization = createInterfaceVisualization(networkNode, interfaceEntry);
                        addInterfaceVisualization(interfaceVisualization);
                    }
                }
            }
        }
    }
}

void InterfaceTableVisualizerBase::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    auto key = std::pair<int, int>(interfaceVisualization->networkNodeId, interfaceVisualization->interfaceId);
    interfaceVisualizations.erase(interfaceVisualizations.find(key));
}

void InterfaceTableVisualizerBase::removeAllInterfaceVisualizations()
{
    std::vector<const InterfaceVisualization *> removedIntefaceVisualizations;
    for (auto it : interfaceVisualizations)
        removedIntefaceVisualizations.push_back(it.second);
    for (auto it : removedIntefaceVisualizations) {
        removeInterfaceVisualization(it);
        delete it;
    }
}

void InterfaceTableVisualizerBase::updateAllInterfaceVisualizations()
{
    removeAllInterfaceVisualizations();
    addAllInterfaceVisualizations();
}

std::string InterfaceTableVisualizerBase::getVisualizationText(const InterfaceEntry *interfaceEntry)
{
    DirectiveResolver directiveResolver(interfaceEntry);
    return format.formatString(&directiveResolver);
}

void InterfaceTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == interfaceCreatedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto interfaceEntry = static_cast<InterfaceEntry *>(object);
            if (interfaceFilter.matches(interfaceEntry)) {
                auto interfaceVisualization = createInterfaceVisualization(networkNode, interfaceEntry);
                addInterfaceVisualization(interfaceVisualization);
            }
        }
    }
    else if (signal == interfaceDeletedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto interfaceEntry = static_cast<InterfaceEntry *>(object);
            if (interfaceFilter.matches(interfaceEntry)) {
                auto interfaceVisualization = getInterfaceVisualization(networkNode, interfaceEntry);
                removeInterfaceVisualization(interfaceVisualization);
            }
        }
    }
    else if (signal == interfaceConfigChangedSignal || signal == interfaceIpv4ConfigChangedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (object != nullptr && nodeFilter.matches(networkNode)) {
            auto interfaceEntryDetails = static_cast<InterfaceEntryChangeDetails *>(object);
            auto interfaceEntry = interfaceEntryDetails->getInterfaceEntry();
            auto fieldId = interfaceEntryDetails->getFieldId();
            if (fieldId == InterfaceEntry::F_IPV4_DATA
#ifdef WITH_IPv4
                    || fieldId == Ipv4InterfaceData::F_IP_ADDRESS || fieldId == Ipv4InterfaceData::F_NETMASK
#endif // WITH_IPv4
                    ) {
                if (interfaceFilter.matches(interfaceEntry)) {
                    auto interfaceVisualization = getInterfaceVisualization(networkNode, interfaceEntry);
                    if (interfaceVisualization == nullptr) {
                        interfaceVisualization = createInterfaceVisualization(networkNode, interfaceEntry);
                        addInterfaceVisualization(interfaceVisualization);
                    }
                    else
                        refreshInterfaceVisualization(interfaceVisualization, interfaceEntry);
                }
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

