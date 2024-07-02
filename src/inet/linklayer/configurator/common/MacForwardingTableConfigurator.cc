//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/common/MacForwardingTableConfigurator.h"

#include "inet/common/MatchableObject.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/common/StreamRedundancyConfigurator.h"
#include "inet/networklayer/arp/ipv4/GlobalArp.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(MacForwardingTableConfigurator);

MacForwardingTableConfigurator::~MacForwardingTableConfigurator()
{
    for (auto it : configurations)
        delete it.second;
}

void MacForwardingTableConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureMacForwardingTables();
        getParentModule()->subscribe(ipv4MulticastChangeSignal, this);
    }
}

void MacForwardingTableConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology))
    TIME(computeMacForwardingTables());
    printElapsedTime("initialize", initializeStartTime);
}

void MacForwardingTableConfigurator::computeMacForwardingTables()
{
    // for each network node as destination
    //   for each network interface of destination
    //   find shortest path from all sources to destination passing through the network interface
    //     for each switch as source
    //       store first outgoing interface of the switch in the MAC forwarding table of the switch
    //       towards the MAC address of the network interface of the destination
    for (int i = 0; i < topology->getNumNodes(); i++) {
        Node *destinationNode = (Node *)topology->getNode(i);
        if (!isBridgeNode(destinationNode)) {
            for (auto destinationInterface : destinationNode->interfaces) {
                if (!destinationInterface->networkInterface->isLoopback() &&
                    !destinationInterface->networkInterface->getMacAddress().isUnspecified())
                {
                    extendConfiguration(destinationNode, destinationInterface, destinationInterface->networkInterface->getMacAddress());
                }
            }
        }
    }
}

void MacForwardingTableConfigurator::extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress)
{
    for (int i = 0; i < destinationNode->getNumInLinks(); i++) {
        auto link = (Link *)destinationNode->getLinkIn(i);
        link->setWeight(link->destinationInterface != destinationInterface ? std::numeric_limits<double>::infinity() : 1);
    }
    topology->calculateWeightedSingleShortestPathsTo(destinationNode);
    for (int i = 0; i < topology->getNumNodes(); i++) {
        Node *sourceNode = (Node *)topology->getNode(i);
        if (sourceNode != destinationNode && isBridgeNode(sourceNode) && sourceNode->getNumPaths() != 0) {
            auto firstLink = (Link *)sourceNode->getPath(0);
            auto firstInterface = static_cast<Interface *>(firstLink->sourceInterface);
            auto interfaceName = firstInterface->networkInterface->getInterfaceName();
            auto moduleId = sourceNode->module->getSubmodule("macTable")->getId();
            auto it = configurations.find(moduleId);
            cValueArray *rules = nullptr;
            if (it == configurations.end())
                rules = configurations[moduleId] = new cValueArray();
            else
                rules = it->second;
            if (findForwardingRule(rules, macAddress, interfaceName) == nullptr) {
                auto rule = new cValueMap();
                rule->set("address", macAddress.str());
                rule->set("interface", interfaceName);
                rules->add(rule);
            }
        }
    }
    for (int i = 0; i < destinationNode->getNumInLinks(); i++) {
        auto link = (Link *)destinationNode->getLinkIn(i);
        link->setWeight(1);
    }
}

cValueMap *MacForwardingTableConfigurator::findForwardingRule(cValueArray *configuration, MacAddress macAddress, std::string interfaceName)
{
    for (int i = 0; i < configuration->size(); i++) {
        auto rule = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        if (rule->get("address").stdstringValue() == macAddress.str() &&
            rule->get("interface").stdstringValue() == interfaceName)
        {
            return rule;
        }
    }
    return nullptr;
}

void MacForwardingTableConfigurator::configureMacForwardingTables() const
{
    for (auto it : configurations) {
        auto macForwardingTable = getSimulation()->getModule(it.first);
        macForwardingTable->par("forwardingTable") = it.second->dup();
    }
}

void MacForwardingTableConfigurator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == ipv4MulticastChangeSignal) {
        auto sourceInfo = check_and_cast<Ipv4MulticastGroupSourceInfo *>(object);
        auto node = static_cast<Node *>(topology->getNodeFor(getContainingNode(sourceInfo->ie)));
        auto interface = findInterface(node, sourceInfo->ie);
        extendConfiguration(node, interface, sourceInfo->groupAddress.mapToMulticastMacAddress());
        configureMacForwardingTables();
    }
}

} // namespace inet

