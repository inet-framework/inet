//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/common/EagerGateScheduleConfigurator.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(EagerGateScheduleConfigurator);

simtime_t simtimeModulo(simtime_t a, simtime_t b)
{
    return SimTime::fromRaw(a.raw() % b.raw());
}

EagerGateScheduleConfigurator::Output *EagerGateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    std::map<NetworkInterface *, std::vector<Slot>> interfaceSchedules;
    auto output = new Output();
    for (auto flow : input.flows) {
        simtime_t startOffset = computeStreamStartOffset(*flow, interfaceSchedules);
        ASSERT(flow->startApplication->module);
        output->applicationStartTimes[flow->startApplication] = startOffset;
        addGateScheduling(*flow, 0, 1, startOffset, interfaceSchedules);
    }
    for (auto flow : input.flows) {
        simtime_t startOffset = output->applicationStartTimes[flow->startApplication];
        int count = gateCycleDuration / flow->startApplication->packetInterval;
        if (gateCycleDuration != count * flow->startApplication->packetInterval)
            throw cRuntimeError("Gate cycle duration must be a multiple of the application packet interval");
        addGateScheduling(*flow, 1, count, startOffset, interfaceSchedules);
    }
    for (auto networkNode : input.networkNodes) {
        for (auto port : networkNode->ports) {
            auto& schedules = output->gateSchedules[port];
            auto& slots = interfaceSchedules[check_and_cast<NetworkInterface *>(port->module)];
            for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
                auto schedule = new Output::Schedule();
                schedule->port = port;
                schedule->gateIndex = gateIndex;
                schedule->cycleStart = 0;
                schedule->cycleDuration = gateCycleDuration;
                for (auto& slot : slots) {
                    if (slot.gateOpenIndex == gateIndex) {
                        simtime_t slotStart = slot.gateOpenTime;
                        simtime_t slotEnd = slot.gateCloseTime;
                        simtime_t slotDuration = slot.gateCloseTime - slot.gateOpenTime;
                        if (slotStart < gateCycleDuration && slotEnd > gateCycleDuration) {
                            Output::Slot scheduleSlot;
                            scheduleSlot.start = slotStart;
                            scheduleSlot.duration = gateCycleDuration - slotStart;
                            schedule->slots.push_back(scheduleSlot);
                            scheduleSlot.start = 0;
                            scheduleSlot.duration = slotDuration - scheduleSlot.duration;
                            schedule->slots.push_back(scheduleSlot);
                        }
                        else {
                            Output::Slot scheduleSlot;
                            scheduleSlot.start = simtimeModulo(slotStart, gateCycleDuration);
                            scheduleSlot.duration = slotDuration;
                            schedule->slots.push_back(scheduleSlot);
                        }
                    }
                }
                auto& slots = schedule->slots;
                std::sort(slots.begin(), slots.end(), [] (const Output::Slot& slot1, const Output::Slot& slot2) {
                    return slot1.start < slot2.start;
                });
                schedules.push_back(schedule);
            }
        }
    }
    return output;
}

simtime_t EagerGateScheduleConfigurator::computeStreamStartOffset(Input::Flow& flow, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const
{
    auto source = flow.startApplication->device->module;
    auto destination = flow.endDevice->module;
    auto pcp = flow.startApplication->pcp;
    bps datarate = flow.startApplication->packetLength / s(flow.startApplication->packetInterval.dbl());
    b packetLength = flow.startApplication->packetLength;
    EV_DEBUG << "Computing start offset for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_ENDL;
    simtime_t startOffset = 0;
    while (true) {
        auto startOffsetShift = computeStartOffsetForPathFragments(flow, source->getFullName(), startOffset, interfaceSchedules);
        if (startOffsetShift == 0)
            break;
        else
            startOffset += startOffsetShift;
    }
    EV_DEBUG << "Setting start offset for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_FIELD(startOffset, startOffset.ustr()) << EV_ENDL;
    return startOffset;
}

simtime_t EagerGateScheduleConfigurator::computeStartOffsetForPathFragments(Input::Flow& flow, std::string startNetworkNodeName, simtime_t startTime, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const
{
    auto destination = flow.endDevice->module;
    b packetLength = flow.startApplication->packetLength;
    simtime_t result = 0;
    std::deque<std::tuple<std::string, simtime_t, std::vector<std::string>>> todos;
    todos.push_back({startNetworkNodeName, startTime, {}});
    while (!todos.empty()) {
        auto todo = todos.front();
        todos.pop_front();
        simtime_t startOffsetShift = 0;
        for (auto pathFragment : flow.pathFragments) {
            if (!strcmp(pathFragment->networkNodes.front()->module->getFullName(), std::get<0>(todo).c_str())) {
                simtime_t nextGateOpenTime = std::get<1>(todo);
                std::vector<std::string> extendedPath = std::get<2>(todo);
                for (int i = 0; i < pathFragment->networkNodes.size() - 1; i++) {
                    auto networkNodeName = pathFragment->networkNodes[i]->module->getFullName();
                    extendedPath.push_back(networkNodeName);
                    auto networkNode = getParentModule()->getSubmodule(networkNodeName);
                    auto node = (Node *)topology->getNodeFor(networkNode);
                    auto link = (Link *)findLinkOut(node, pathFragment->networkNodes[i + 1]->module->getFullName());
                    auto interface = link->sourceInterface;
                    auto networkInterface = interface->networkInterface;
                    auto& interfaceSchedule = interfaceSchedules[networkInterface];
                    bps interfaceDatarate = bps(networkInterface->getDatarate());
                    simtime_t transmissionDuration = s(packetLength / interfaceDatarate).get();
                    simtime_t interFrameGap = s(b(96) / interfaceDatarate).get();
                    auto channel = dynamic_cast<cDatarateChannel *>(networkInterface->getTxTransmissionChannel());
                    simtime_t propagationDelay = channel != nullptr ? channel->getDelay() : 0;
                    simtime_t gateOpenDuration = transmissionDuration;
                    simtime_t gateOpenTime = nextGateOpenTime;
                    simtime_t gateCloseTime = gateOpenTime + gateOpenDuration;
                    for (int i = 0; i < interfaceSchedule.size(); i++) {
                        if (interfaceSchedule[i].gateCloseTime + interFrameGap <= gateOpenTime || gateCloseTime + interFrameGap <= interfaceSchedule[i].gateOpenTime)
                            continue;
                        else {
                            gateOpenTime = interfaceSchedule[i].gateCloseTime + interFrameGap;
                            gateCloseTime = gateOpenTime + gateOpenDuration;
                            i = 0;
                        }
                    }
                    simtime_t gateOpenDelay = gateOpenTime - nextGateOpenTime;
                    if (gateOpenDelay != 0) {
                        startOffsetShift += gateOpenDelay;
                        break;
                    }
                    nextGateOpenTime = gateCloseTime + propagationDelay;
                }
                auto endNetworkNodeName = pathFragment->networkNodes.back()->module->getFullName();
                if (endNetworkNodeName != destination->getFullName() && std::find(extendedPath.begin(), extendedPath.end(), endNetworkNodeName) == extendedPath.end())
                    todos.push_back({endNetworkNodeName, nextGateOpenTime, extendedPath});
            }
        }
        result += startOffsetShift;
    }
    return result;
};

void EagerGateScheduleConfigurator::addGateScheduling(Input::Flow& flow, int startIndex, int endIndex, simtime_t startOffset, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const
{
    auto source = flow.startApplication->device->module;
    auto destination = flow.endDevice->module;
    auto pcp = flow.startApplication->pcp;
    bps datarate = flow.startApplication->packetLength / s(flow.startApplication->packetInterval.dbl());
    b packetLength = flow.startApplication->packetLength;
    EV_DEBUG << "Allocating gate scheduling for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_FIELD(startOffset, startOffset.ustr()) << EV_FIELD(startIndex) << EV_FIELD(endIndex) << EV_ENDL;
    for (int index = startIndex; index < endIndex; index++) {
        simtime_t startTime = startOffset + index * flow.startApplication->packetInterval;
        addGateSchedulingForPathFragments(flow, source->getFullName(), startTime, index, interfaceSchedules);
    }
}

void EagerGateScheduleConfigurator::addGateSchedulingForPathFragments(Input::Flow& flow, std::string startNetworkNodeName, simtime_t startTime, int index, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const
{
    auto destination = flow.endDevice->module;
    auto pcp = flow.startApplication->pcp;
    b packetLength = flow.startApplication->packetLength;
    std::deque<std::tuple<std::string, simtime_t, std::vector<std::string>>> todos;
    todos.push_back({startNetworkNodeName, startTime, {}});
    while (!todos.empty()) {
        auto todo = todos.front();
        todos.pop_front();
        for (auto pathFragment : flow.pathFragments) {
            if (!strcmp(pathFragment->networkNodes.front()->module->getFullName(), std::get<0>(todo).c_str())) {
                simtime_t nextGateOpenTime = std::get<1>(todo);
                std::vector<std::string> extendedPath = std::get<2>(todo);
                for (int i = 0; i < pathFragment->networkNodes.size() - 1; i++) {
                    auto networkNodeName = pathFragment->networkNodes[i]->module->getFullName();
                    extendedPath.push_back(networkNodeName);
                    auto networkNode = getParentModule()->getSubmodule(networkNodeName);
                    auto node = (Node *)topology->getNodeFor(networkNode);
                    auto link = (Link *)findLinkOut(node, pathFragment->networkNodes[i + 1]->module->getFullName());
                    auto interface = link->sourceInterface;
                    auto networkInterface = interface->networkInterface;
                    auto& interfaceSchedule = interfaceSchedules[networkInterface];
                    bps interfaceDatarate = bps(networkInterface->getDatarate());
                    simtime_t transmissionDuration = s(packetLength / interfaceDatarate).get();
                    simtime_t interFrameGap = s(b(96) / interfaceDatarate).get();
                    auto channel = dynamic_cast<cDatarateChannel *>(networkInterface->getTxTransmissionChannel());
                    simtime_t propagationDelay = channel != nullptr ? channel->getDelay() : 0;
                    simtime_t gateOpenDuration = transmissionDuration;
                    simtime_t gateOpenTime = nextGateOpenTime;
                    simtime_t gateCloseTime = gateOpenTime + gateOpenDuration;
                    for (int i = 0; i < interfaceSchedule.size(); i++) {
                        if (interfaceSchedule[i].gateCloseTime + interFrameGap <= gateOpenTime || gateCloseTime + interFrameGap <= interfaceSchedule[i].gateOpenTime)
                            continue;
                        else {
                            gateOpenTime = interfaceSchedule[i].gateCloseTime + interFrameGap;
                            gateCloseTime = gateOpenTime + gateOpenDuration;
                            i = 0;
                        }
                    }
                    simtime_t extraDelay = gateOpenTime - nextGateOpenTime;
                    EV_DEBUG << "Extending gate scheduling for stream reservation" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(pcp) << EV_FIELD(interfaceDatarate) << EV_FIELD(packetLength) << EV_FIELD(index) << EV_FIELD(startTime, startTime.ustr()) << EV_FIELD(gateOpenTime, gateOpenTime.ustr()) << EV_FIELD(gateCloseTime, gateCloseTime.ustr()) << EV_FIELD(gateOpenDuration, gateOpenDuration.ustr()) << EV_FIELD(extraDelay, extraDelay.ustr()) << EV_ENDL;
                    if (gateCloseTime > startTime + gateCycleDuration)
                        throw cRuntimeError("Gate scheduling doesn't fit into cycle duration");
                    Slot entry;
                    entry.gateOpenIndex = flow.gateIndex;
                    entry.gateOpenTime = gateOpenTime;
                    entry.gateCloseTime = gateCloseTime;
                    interfaceSchedule.push_back(entry);
                    nextGateOpenTime = gateCloseTime + propagationDelay;
                }
                auto endNetworkNodeName = pathFragment->networkNodes.back()->module->getFullName();
                if (endNetworkNodeName == destination->getFullName() && nextGateOpenTime - startTime > flow.startApplication->maxLatency)
                    throw cRuntimeError("Cannot fit scheduling int maximum allowed latency");
                if (endNetworkNodeName != destination->getFullName() && std::find(extendedPath.begin(), extendedPath.end(), endNetworkNodeName) == extendedPath.end())
                    todos.push_back({endNetworkNodeName, nextGateOpenTime, extendedPath});
            }
        }
    }
}

} // namespace inet

