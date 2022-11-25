//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

#include "inet/common/PatternMatcher.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {

void GateScheduleConfiguratorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void GateScheduleConfiguratorBase::handleParameterChange(const char *name)
{
    if (!strcmp(name, "configuration")) {
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
        clearConfiguration();
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void GateScheduleConfiguratorBase::clearConfiguration()
{
    if (topology != nullptr)
        topology->clear();
    delete gateSchedulingInput;
    gateSchedulingInput = nullptr;
    delete gateSchedulingOutput;
    gateSchedulingOutput = nullptr;
}

void GateScheduleConfiguratorBase::computeConfiguration()
{
    long startTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology));
    TIME(gateSchedulingInput = createGateSchedulingInput());
    TIME(gateSchedulingOutput = computeGateScheduling(*gateSchedulingInput));
    printElapsedTime("computeConfiguration", startTime);
}

GateScheduleConfiguratorBase::Input *GateScheduleConfiguratorBase::createGateSchedulingInput() const
{
    auto input = new Input();
    addDevices(*input);
    addSwitches(*input);
    addPorts(*input);
    addFlows(*input);
    return input;
}

void GateScheduleConfiguratorBase::addDevices(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        if (!isBridgeNode(node)) {
            auto device = new Input::Device();
            device->module = node->module;
            input.devices.push_back(device);
            input.networkNodes.push_back(device);
        }
    }
}

void GateScheduleConfiguratorBase::addSwitches(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        if (isBridgeNode(node)) {
            auto switch_ = new Input::Switch();
            switch_->module = node->module;
            input.switches.push_back(switch_);
            input.networkNodes.push_back(switch_);
        }
    }
}

void GateScheduleConfiguratorBase::addPorts(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = input.getNetworkNode(node->module);
        for (auto interface : node->interfaces) {
            auto networkInterface = interface->networkInterface;
            if (!networkInterface->isLoopback()) {
                auto subqueue = networkInterface->findModuleByPath(".macLayer.queue.queue[0]");
                auto port = new Input::Port();
                port->numGates = subqueue != nullptr ? subqueue->getVectorSize() : -1;
                port->module = interface->networkInterface;
                port->datarate = bps(interface->networkInterface->getDatarate());
                port->propagationTime = check_and_cast<cDatarateChannel *>(interface->networkInterface->getTxTransmissionChannel())->getDelay();
                port->maxPacketLength = B(interface->networkInterface->getMtu());
                port->guardBand = s(port->maxPacketLength / port->datarate).get();
                port->maxCycleTime = gateCycleDuration;
                port->maxSlotDuration = gateCycleDuration;
                port->cutthroughSwitchingEnabled = true; // TODO: extract from network interface!
                port->startNode = networkNode;
                networkNode->ports.push_back(port);
                input.ports.push_back(port);
            }
        }
    }
    for (auto networkNode : input.networkNodes) {
        auto node = check_and_cast<Node *>(topology->getNodeFor(networkNode->module));
        for (auto port : networkNode->ports) {
            auto networkInterface = check_and_cast<NetworkInterface *>(port->module);
            auto link = findLinkOut(findInterface(node, networkInterface));
            auto linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
            auto remoteNode = check_and_cast<Node *>(linkOut->getLinkOutRemoteNode());
            port->endNode = *std::find_if(input.networkNodes.begin(), input.networkNodes.end(), [&] (const auto& networkNode) {
                return networkNode->module == remoteNode->module;
            });
            port->otherPort = *std::find_if(input.ports.begin(), input.ports.end(), [&] (const auto& otherPort) {
                return otherPort->module == link->destinationInterface->networkInterface;
            });
            ASSERT(port->endNode);
            ASSERT(port->otherPort);
        }
    }
}

void GateScheduleConfiguratorBase::addFlows(Input& input) const
{
    int flowIndex = 0;
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
        for (int i = 0; i < topology->getNumNodes(); i++) {
            auto sourceNode = (Node *)topology->getNode(i);
            cModule *source = sourceNode->module;
            for (int j = 0; j < topology->getNumNodes(); j++) {
                auto destinationNode = (Node *)topology->getNode(j);
                cModule *destination = destinationNode->module;
                PatternMatcher sourceMatcher(entry->get("source").stringValue(), true, false, false);
                PatternMatcher destinationMatcher(entry->get("destination").stringValue(), true, false, false);
                if (sourceMatcher.matches(sourceNode->module->getFullPath().c_str()) &&
                    destinationMatcher.matches(destinationNode->module->getFullPath().c_str()))
                {
                    int pcp = entry->get("pcp").intValue();
                    int gateIndex = entry->get("gateIndex").intValue();
                    b packetLength = b(entry->get("packetLength").doubleValueInUnit("b"));
                    b cutthroughSwitchingHeaderSize = entry->containsKey("cutthroughSwitchingHeaderSize") ? b(entry->get("cutthroughSwitchingHeaderSize").doubleValueInUnit("b")) : b(0);
                    simtime_t packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
                    simtime_t maxLatency = entry->containsKey("maxLatency") ? entry->get("maxLatency").doubleValueInUnit("s") : -1;
                    simtime_t maxJitter = entry->containsKey("maxJitter") ? entry->get("maxJitter").doubleValueInUnit("s") : 0;
                    bps datarate = packetLength / s(packetInterval.dbl());
                    auto startDevice = input.getDevice(source);
                    auto endDevice = input.getDevice(destination);
                    auto startApplication = new Input::Application();
                    auto startApplicationModule = startDevice->module->getModuleByPath((std::string(".") + std::string(entry->get("application").stringValue())).c_str());
                    if (startApplicationModule == nullptr)
                        throw cRuntimeError("Cannot find flow start application, path = %s", entry->get("application").stringValue());
                    startApplication->module = startApplicationModule;
                    startApplication->device = startDevice;
                    startApplication->pcp = pcp;
                    startApplication->packetLength = packetLength;
                    startApplication->packetInterval = packetInterval;
                    startApplication->maxLatency = maxLatency;
                    startApplication->maxJitter = maxJitter;
                    input.applications.push_back(startApplication);
                    EV_DEBUG << "Adding flow from configuration" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(datarate) << EV_FIELD(maxLatency, maxLatency.ustr()) << EV_FIELD(maxJitter, maxJitter.ustr()) << EV_ENDL;
                    auto flow = new Input::Flow();
                    flow->name = entry->containsKey("name") ? entry->get("name").stringValue() : (std::string("flow") + std::to_string(flowIndex++)).c_str();
                    flow->gateIndex = gateIndex;
                    flow->cutthroughSwitchingHeaderSize = cutthroughSwitchingHeaderSize;
                    flow->startApplication = startApplication;
                    flow->endDevice = endDevice;
                    cValueArray *pathFragments;
                    if (entry->containsKey("pathFragments"))
                        pathFragments = check_and_cast<cValueArray *>(entry->get("pathFragments").objectValue());
                    else {
                        auto pathFragment = new cValueArray();
                        for (auto node : computeShortestNodePath(sourceNode, destinationNode))
                            pathFragment->add(node->module->getFullName());
                        pathFragments = new cValueArray();
                        pathFragments->add(pathFragment);
                    }
                    for (int l = 0; l < pathFragments->size(); l++) {
                        auto path = new Input::PathFragment();
                        auto pathFragment = check_and_cast<cValueArray *>(pathFragments->get(l).objectValue());
                        for (int m = 0; m < pathFragment->size(); m++) {
                            for (auto networkNode : input.networkNodes) {
                                auto name = pathFragment->get(m).stdstringValue();
                                int index = name.find('.');
                                auto nodeName = index != std::string::npos ? name.substr(0, index) : name;
                                auto interfaceName = index != std::string::npos ? name.substr(index + 1) : "";
                                if (networkNode->module->getFullName() == nodeName) {
                                    if (m != pathFragment->size() - 1) {
                                        auto startNode = networkNode;
                                        auto endNodeName = pathFragment->get(m + 1).stdstringValue();
                                        int index = endNodeName.find('.');
                                        endNodeName = index != std::string::npos ? endNodeName.substr(0, index) : endNodeName;
                                        auto outputPort = *std::find_if(startNode->ports.begin(), startNode->ports.end(), [&] (const auto& port) {
                                            return port->endNode->module->getFullName() == endNodeName && (interfaceName == "" || interfaceName == check_and_cast<NetworkInterface *>(port->module)->getInterfaceName());
                                        });
                                        path->outputPorts.push_back(outputPort);
                                        path->inputPorts.push_back(outputPort->otherPort);
                                    }
                                    path->networkNodes.push_back(networkNode);
                                    break;
                                }
                            }
                        }
                        flow->pathFragments.push_back(path);
                    }
                    if (!entry->containsKey("pathFragments"))
                        delete pathFragments;
                    input.flows.push_back(flow);
                }
            }
        }
    }
    std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1, const Input::Flow *r2) {
        return r1->startApplication->pcp > r2->startApplication->pcp;
    });
}

void GateScheduleConfiguratorBase::configureGateScheduling()
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        for (auto interface : node->interfaces) {
            auto queue = interface->networkInterface->findModuleByPath(".macLayer.queue");
            if (queue != nullptr) {
                for (cModule::SubmoduleIterator it(queue); !it.end(); ++it) {
                    cModule *gate = *it;
                    if (dynamic_cast<queueing::PeriodicGate *>(gate) != nullptr)
                        configureGateScheduling(networkNode, gate, interface);
                }
            }
        }
    }
}

void GateScheduleConfiguratorBase::configureGateScheduling(cModule *networkNode, cModule *gate, Interface *interface)
{
    auto networkInterface = interface->networkInterface;
    simtime_t offset = 0;
    simtime_t slotEnd = 0;
    int gateIndex = gate->getIndex();
    auto port = gateSchedulingInput->getPort(networkInterface);
    auto it = gateSchedulingOutput->gateSchedules.find(port);
    if (it == gateSchedulingOutput->gateSchedules.end())
        throw cRuntimeError("Cannot find schedule for interface, interface = %s", networkInterface->getInterfaceFullPath().c_str());
    auto& schedules = it->second;
    if (gateIndex >= schedules.size())
        throw cRuntimeError("Cannot find schedule for traffic class, interface = %s, gate index = %d", port->module->getFullPath().c_str(), gateIndex);
    auto schedule = schedules[gateIndex];
    bool initiallyOpen = !schedule->open;
    cValueArray *durations = new cValueArray();
    for (auto& slot : schedule->slots) {
        simtime_t slotStart = slot.start;
        simtime_t slotDuration = slot.duration;
        if (slotStart < 0 || slotStart + slotDuration > gateCycleDuration)
            throw cRuntimeError("Invalid slot start and/or duration");
        if (slotStart == 0)
            initiallyOpen = schedule->open;
        else {
            simtime_t duration = slotStart - slotEnd;
            ASSERT(duration >= 0);
            durations->add(cValue(duration.dbl(), "s"));
        }
        durations->add(cValue(slotDuration.dbl(), "s"));
        slotEnd = slotStart + slotDuration;
    }
    simtime_t remainingDuration = schedule->cycleDuration - slotEnd;
    if (slotEnd != 0) {
        if (remainingDuration != 0)
            durations->add(cValue(remainingDuration.dbl(), "s"));
        if (durations->size() % 2 != 0) {
            if (durations->size() > 1) {
                double delta = durations->get(durations->size() - 1).doubleValueInUnit("s");
                durations->set(0, cValue(durations->get(0).doubleValueInUnit("s") + delta, "s"));
                offset += delta;
            }
            durations->erase(durations->size() - 1);
        }
    }
    EV_DEBUG << "Configuring gate scheduling parameters" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(gate) << EV_FIELD(initiallyOpen) << EV_FIELD(offset) << EV_FIELD(durations) << EV_ENDL;
    gate->par("initiallyOpen") = initiallyOpen;
    gate->par("offset") = offset.dbl();
    cPar& durationsPar = gate->par("durations");
    durationsPar.copyIfShared();
    durationsPar.setObjectValue(durations);
}

void GateScheduleConfiguratorBase::configureApplicationOffsets()
{
    for (auto& it : gateSchedulingOutput->applicationStartTimes) {
        auto startOffset = it.second;
        auto applicationModule = it.first->module;
        EV_DEBUG << "Setting initial packet production offset for application source" << EV_FIELD(applicationModule) << EV_FIELD(startOffset) << EV_ENDL;
        auto sourceModule = applicationModule->getSubmodule("source");
        sourceModule->par("initialProductionOffset") = startOffset.dbl();
    }
}

} // namespace inet

