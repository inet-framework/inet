//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/configurator/gatescheduling/base/GateSchedulingConfiguratorBase.h"

#include "inet/common/PatternMatcher.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {

void GateSchedulingConfiguratorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    if (stage == INITSTAGE_QUEUEING) {
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void GateSchedulingConfiguratorBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "configuration")) {
            configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
            clearConfiguration();
            computeConfiguration();
            configureGateScheduling();
            configureApplicationOffsets();
        }
    }
}

void GateSchedulingConfiguratorBase::clearConfiguration()
{
    topology.clear();
    delete gateSchedulingInput;
    gateSchedulingInput = nullptr;
    delete gateSchedulingOutput;
    gateSchedulingOutput = nullptr;
}

void GateSchedulingConfiguratorBase::computeConfiguration()
{
    long startTime = clock();
    TIME(extractTopology(topology));
    TIME(gateSchedulingInput = createGateSchedulingInput());
    TIME(gateSchedulingOutput = computeGateScheduling(*gateSchedulingInput));
    printElapsedTime("computeConfiguration", startTime);
}

GateSchedulingConfiguratorBase::Input *GateSchedulingConfiguratorBase::createGateSchedulingInput() const
{
    auto input = new Input();
    addDevices(*input);
    addSwitches(*input);
    addPorts(*input);
    addFlows(*input);
    return input;
}

void GateSchedulingConfiguratorBase::addDevices(Input& input) const
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        if (!isBridgeNode(node)) {
            auto device = new Input::Device();
            device->module = node->module;
            input.devices.push_back(device);
            input.networkNodes.push_back(device);
        }
    }
}

void GateSchedulingConfiguratorBase::addSwitches(Input& input) const
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        if (isBridgeNode(node)) {
            auto switch_ = new Input::Switch();
            switch_->module = node->module;
            input.switches.push_back(switch_);
            input.networkNodes.push_back(switch_);
        }
    }
}

void GateSchedulingConfiguratorBase::addPorts(Input& input) const
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = input.getNetworkNode(node->module);
        for (auto interfaceInfo : node->interfaceInfos) {
            auto networkInterface = interfaceInfo->networkInterface;
            if (!networkInterface->isLoopback()) {
                auto subqueue = networkInterface->findModuleByPath(".macLayer.queue.queue[0]");
                auto port = new Input::Port();
                port->numPriorities = subqueue != nullptr ? subqueue->getVectorSize() : -1;
                port->module = interfaceInfo->networkInterface;
                port->datarate = bps(interfaceInfo->networkInterface->getDatarate());
                port->propagationTime = check_and_cast<cDatarateChannel *>(interfaceInfo->networkInterface->getTxTransmissionChannel())->getDelay();
                port->maxPacketLength = B(interfaceInfo->networkInterface->getMtu());
                port->guardBand = s(port->maxPacketLength / port->datarate).get();
                port->maxCycleTime = gateCycleDuration;
                port->maxSlotDuration = gateCycleDuration;
                port->startNode = networkNode;
                networkNode->ports.push_back(port);
                input.ports.push_back(port);
            }
        }
    }
    for (auto switch_ : input.switches) {
        auto node = check_and_cast<Node *>(topology.getNodeFor(switch_->module));
        for (auto port : switch_->ports) {
            auto networkInterface = check_and_cast<NetworkInterface *>(port->module);
            auto linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
            auto remoteNode = check_and_cast<Node *>(linkOut->getRemoteNode());
            for (auto networkNode : input.networkNodes) {
                if (networkNode->module == remoteNode->module)
                    port->endNode = networkNode;
            }
        }
    }
}

void GateSchedulingConfiguratorBase::addFlows(Input& input) const
{
    int flowIndex = 0;
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
        for (int i = 0; i < topology.getNumNodes(); i++) {
            auto sourceNode = (Node *)topology.getNode(i);
            cModule *source = sourceNode->module;
            for (int j = 0; j < topology.getNumNodes(); j++) {
                auto destinationNode = (Node *)topology.getNode(j);
                cModule *destination = destinationNode->module;
                PatternMatcher sourceMatcher(entry->get("source").stringValue(), true, false, false);
                PatternMatcher destinationMatcher(entry->get("destination").stringValue(), true, false, false);
                if (sourceMatcher.matches(sourceNode->module->getFullPath().c_str()) &&
                    destinationMatcher.matches(destinationNode->module->getFullPath().c_str()))
                {
                    int priority = entry->get("priority").intValue();
                    b packetLength = b(entry->get("packetLength").doubleValueInUnit("b"));
                    simtime_t packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
                    simtime_t maxLatency = entry->get("maxLatency").doubleValueInUnit("s");
                    bps datarate = packetLength / s(packetInterval.dbl());
                    auto startDevice = input.getDevice(source);
                    auto endDevice = input.getDevice(destination);
                    auto startApplication = new Input::Application();
                    auto startApplicationModule = startDevice->module->getModuleByPath((std::string(".") + std::string(entry->get("application").stringValue())).c_str());
                    if (startApplicationModule == nullptr)
                        throw cRuntimeError("Cannot find flow start application, path = %s", entry->get("application").stringValue());
                    startApplication->module = startApplicationModule;
                    startApplication->device = startDevice;
                    startApplication->priority = priority;
                    startApplication->packetLength = packetLength;
                    startApplication->packetInterval = packetInterval;
                    startApplication->maxLatency = maxLatency;
                    input.applications.push_back(startApplication);
                    EV_DEBUG << "Adding flow from configuration" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(priority) << EV_FIELD(packetLength) << EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(datarate) << EV_FIELD(maxLatency, maxLatency.ustr()) << EV_ENDL;
                    auto flow = new Input::Flow();
                    flow->name = entry->containsKey("name") ? entry->get("name").stringValue() : (std::string("flow") + std::to_string(flowIndex++)).c_str();
                    flow->startApplication = startApplication;
                    flow->endDevice = endDevice;
                    if (entry->containsKey("pathFragments")) {
                        auto pathFragments = check_and_cast<cValueArray *>(entry->get("pathFragments").objectValue());
                        for (int l = 0; l < pathFragments->size(); l++) {
                            auto path = new Input::PathFragment();
                            auto pathFragment = check_and_cast<cValueArray *>(pathFragments->get(l).objectValue());
                            for (int m = 0; m < pathFragment->size(); m++) {
                                for (auto networkNode : input.networkNodes) {
                                    if (!strcmp(networkNode->module->getFullName(), pathFragment->get(m).stringValue())) {
                                        path->networkNodes.push_back(networkNode);
                                        break;
                                    }
                                }
                            }
                            flow->pathFragments.push_back(path);
                        }
                    }
                    else {
                        auto pathFragment = new Input::PathFragment();
                        auto path = computeShortestPath(sourceNode, destinationNode);
                        for (auto node : path) {
                            for (auto networkNode : input.networkNodes) {
                                if (networkNode->module == node->module) {
                                    pathFragment->networkNodes.push_back(networkNode);
                                    break;
                                }
                            }
                        }
                        flow->pathFragments.push_back(pathFragment);
                    }
                    input.flows.push_back(flow);
                }
            }
        }
    }
    std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1, const Input::Flow *r2) {
        return r1->startApplication->priority < r2->startApplication->priority;
    });
}

void GateSchedulingConfiguratorBase::configureGateScheduling()
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        for (auto interfaceInfo : node->interfaceInfos) {
            auto queue = interfaceInfo->networkInterface->findModuleByPath(".macLayer.queue");
            if (queue != nullptr) {
                for (cModule::SubmoduleIterator it(queue); !it.end(); ++it) {
                    cModule *gate = *it;
                    if (dynamic_cast<queueing::PeriodicGate *>(gate) != nullptr)
                        configureGateScheduling(networkNode, gate, interfaceInfo);
                }
            }
        }
    }
}

void GateSchedulingConfiguratorBase::configureGateScheduling(cModule *networkNode, cModule *gate, InterfaceInfo *interfaceInfo)
{
    auto networkInterface = interfaceInfo->networkInterface;
    bool initiallyOpen = false;
    simtime_t offset = 0;
    simtime_t slotEnd = 0;
    int priority = gate->getIndex();
    auto port = gateSchedulingInput->getPort(networkInterface);
    auto it = gateSchedulingOutput->gateSchedules.find(port);
    if (it == gateSchedulingOutput->gateSchedules.end())
        throw cRuntimeError("Cannot find schedule for interface, interface = %s", networkInterface->getInterfaceFullPath().c_str());
    auto& schedules = it->second;
    if (priority >= schedules.size())
        throw cRuntimeError("Cannot find schedule for priority, interface = %s, priority = %d", port->module->getFullPath().c_str(), priority);
    auto schedule = schedules[priority];
    cValueArray *durations = new cValueArray();
    for (auto& slot : schedule->slots) {
        simtime_t slotStart = slot.start;
        simtime_t slotDuration = slot.duration;
        if (slotStart < 0 || slotStart + slotDuration > gateCycleDuration)
            throw cRuntimeError("Invalid slot start and/or duration");
        if (slotStart == 0)
            initiallyOpen = true;
        else {
            simtime_t duration = slotStart - slotEnd;
            ASSERT(duration >= 0);
            durations->add(cValue(duration.dbl(), "s"));
        }
        durations->add(cValue(slotDuration.dbl(), "s"));
        slotEnd = slotStart + slotDuration;
    }
    simtime_t remainingDuration = gateCycleDuration - slotEnd;
    if (slotEnd != 0) {
        if (remainingDuration != 0)
            durations->add(cValue(remainingDuration.dbl(), "s"));
        if (durations->size() % 2 != 0)
            durations->add(cValue(0, "s"));
    }
    EV_DEBUG << "Configuring gate scheduling parameters" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(gate) << EV_FIELD(initiallyOpen) << EV_FIELD(offset) << EV_FIELD(durations) << EV_ENDL;
    gate->par("initiallyOpen") = initiallyOpen;
    gate->par("offset") = offset.dbl();
    cPar& durationsPar = gate->par("durations");
    durationsPar.copyIfShared();
    durationsPar.setObjectValue(durations);
}

void GateSchedulingConfiguratorBase::configureApplicationOffsets()
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

