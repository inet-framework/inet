//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#ifdef INET_WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#endif // INET_WITH_NEXTHOP
#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // INET_WITH_IPv4
#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // INET_WITH_IPv6
#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"

namespace inet {

namespace visualizer {

InterfaceTableVisualizerBase::InterfaceVisualization::InterfaceVisualization(int networkNodeId, int networkNodeGateId, int interfaceId) :
    networkNodeId(networkNodeId),
    networkNodeGateId(networkNodeGateId),
    interfaceId(interfaceId)
{
}

std::string InterfaceTableVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'N':
            return networkInterface->getInterfaceName();
        case 'm':
            return networkInterface->getMacAddress().str();
        case 'l': // TODO Ipv4 or Ipv6
#ifdef INET_WITH_IPv4
            if (auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>())
                return std::to_string(ipv4Data->getNetmask().getNetmaskLength());
#endif // INET_WITH_IPv4
            return "";
        case '4':
#ifdef INET_WITH_IPv4
            if (auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>())
                return ipv4Data->getIPAddress().str();
#endif // INET_WITH_IPv4
            return "";
        case '6':
#ifdef INET_WITH_IPv6
            if (auto ipv6Data = networkInterface->findProtocolData<Ipv6InterfaceData>())
                return ipv6Data->getPreferredAddress().str();
#endif // INET_WITH_IPv6
            return "";
        case 'a':
            if (false) ;
#ifdef INET_WITH_IPv4
            else if (auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>())
                return ipv4Data->getIPAddress().str();
#endif // INET_WITH_IPv4
#ifdef INET_WITH_IPv6
            else if (auto ipv6Data = networkInterface->findProtocolData<Ipv6InterfaceData>())
                return ipv6Data->getPreferredAddress().str();
#endif // INET_WITH_IPv6
#ifdef INET_WITH_NEXTHOP
            else if (auto nextHopData = networkInterface->findProtocolData<NextHopInterfaceData>())
                return nextHopData->getAddress().str();
#endif // INET_WITH_NEXTHOP
            return "";
        case 'g':
#ifdef INET_WITH_NEXTHOP
            if (auto nextHopData = networkInterface->findProtocolData<NextHopInterfaceData>())
                return nextHopData->getAddress().str();
#endif // INET_WITH_NEXTHOP
            return "";
        case 'n':
            return networkInterface->getNetworkAddress().str();
        case 't':
            switch (networkInterface->getState()) {
                case NetworkInterface::UP: return "up"; break;
                case NetworkInterface::DOWN: return "down"; break;
                case NetworkInterface::GOING_UP: return "going up"; break;
                case NetworkInterface::GOING_DOWN: return "going down"; break;
                default: throw cRuntimeError("Unknown interface state");
            }
            break;
        case 'i':
            return networkInterface->str();
        case 's':
            return networkInterface->str();
        case '/':
            return networkInterface->getNetworkAddress().isUnspecified() ? "" : "/";
        case '\\':
            return networkInterface->getNodeOutputGateId() == -1 ? "" : "\n";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void InterfaceTableVisualizerBase::preDelete(cComponent *root)
{
    if (displayInterfaceTables) {
        unsubscribe();
        removeAllInterfaceVisualizations();
    }
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
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    else if (!strcmp(name, "format"))
        format.parseFormat(par("format"));
    updateAllInterfaceVisualizations();
}

void InterfaceTableVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(interfaceCreatedSignal, this);
    visualizationSubjectModule->subscribe(interfaceDeletedSignal, this);
    visualizationSubjectModule->subscribe(interfaceConfigChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceStateChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceIpv6ConfigChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceGnpConfigChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceClnsConfigChangedSignal, this);
}

void InterfaceTableVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(interfaceCreatedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceDeletedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceConfigChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceIpv4ConfigChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceIpv6ConfigChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceGnpConfigChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceClnsConfigChangedSignal, this);
    }
}

cModule *InterfaceTableVisualizerBase::getNetworkNode(const InterfaceVisualization *interfaceVisualization)
{
    return getSimulation()->getModule(interfaceVisualization->networkNodeId);
}

cGate *InterfaceTableVisualizerBase::getOutputGate(cModule *networkNode, NetworkInterface *networkInterface)
{
    if (networkInterface->getNodeOutputGateId() == -1)
        return nullptr;
    cGate *outputGate = networkNode->gate(networkInterface->getNodeOutputGateId());
    if (outputGate == nullptr || outputGate->getChannel() == nullptr)
        return nullptr;
    else
        return outputGate;
}

cGate *InterfaceTableVisualizerBase::getOutputGate(const InterfaceVisualization *interfaceVisualization)
{
    if (interfaceVisualization->networkNodeGateId == -1)
        return nullptr;
    else {
        auto networkNode = getNetworkNode(interfaceVisualization);
        return networkNode != nullptr ? networkNode->gate(interfaceVisualization->networkNodeGateId) : nullptr;
    }
}

const InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableVisualizerBase::getInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    auto key = std::pair<int, int>(networkNode->getId(), networkInterface->getInterfaceId());
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
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
            L3AddressResolver addressResolver;
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                    auto networkInterface = interfaceTable->getInterface(i);
                    if (networkInterface != nullptr && interfaceFilter.matches(networkInterface)) {
                        auto interfaceVisualization = createInterfaceVisualization(networkNode, networkInterface);
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

std::string InterfaceTableVisualizerBase::getVisualizationText(const NetworkInterface *networkInterface)
{
    DirectiveResolver directiveResolver(networkInterface);
    return format.formatString(&directiveResolver);
}

void InterfaceTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == interfaceCreatedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto networkInterface = static_cast<NetworkInterface *>(object);
            if (networkInterface->getInterfaceId() != -1 && interfaceFilter.matches(networkInterface)) {
                auto interfaceVisualization = createInterfaceVisualization(networkNode, networkInterface);
                addInterfaceVisualization(interfaceVisualization);
            }
        }
    }
    else if (signal == interfaceDeletedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto networkInterface = static_cast<NetworkInterface *>(object);
            if (networkInterface->getInterfaceId() != -1 && interfaceFilter.matches(networkInterface)) {
                auto interfaceVisualization = getInterfaceVisualization(networkNode, networkInterface);
                removeInterfaceVisualization(interfaceVisualization);
                delete interfaceVisualization;
            }
        }
    }
    else if (signal == interfaceConfigChangedSignal
            || signal == interfaceIpv4ConfigChangedSignal
            || signal == interfaceIpv6ConfigChangedSignal
            || signal == interfaceGnpConfigChangedSignal
            || signal == interfaceClnsConfigChangedSignal
            || signal == interfaceStateChangedSignal) {
        auto networkNode = getContainingNode(static_cast<cModule *>(source));
        if (object != nullptr && nodeFilter.matches(networkNode)) {
            auto networkInterfaceDetails = static_cast<NetworkInterfaceChangeDetails *>(object);
            auto networkInterface = networkInterfaceDetails->getNetworkInterface();
            auto fieldId = networkInterfaceDetails->getFieldId();
            if ((signal == interfaceConfigChangedSignal && fieldId == NetworkInterface::F_IPV4_DATA)
#ifdef INET_WITH_IPv4
                    || (signal == interfaceIpv4ConfigChangedSignal && (fieldId == Ipv4InterfaceData::F_IP_ADDRESS || fieldId == Ipv4InterfaceData::F_NETMASK))
#endif // INET_WITH_IPv4

                    || (signal == interfaceIpv6ConfigChangedSignal)
                    || signal == interfaceGnpConfigChangedSignal
                    || signal == interfaceClnsConfigChangedSignal

                    || (signal == interfaceStateChangedSignal && (fieldId == NetworkInterface::F_STATE || fieldId == NetworkInterface::F_CARRIER)))
            {
                if (networkInterface->getInterfaceId() != -1 && interfaceFilter.matches(networkInterface)) {
                    auto interfaceVisualization = getInterfaceVisualization(networkNode, networkInterface);
                    if (interfaceVisualization == nullptr) {
                        interfaceVisualization = createInterfaceVisualization(networkNode, networkInterface);
                        addInterfaceVisualization(interfaceVisualization);
                    }
                    else
                        refreshInterfaceVisualization(interfaceVisualization, networkInterface);
                }
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

