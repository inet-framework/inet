//
// Copyright (C) 2021 OpenSim Ltd. and the original authors
//
// This file is partly copied from the following project with the explicit
// permission from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/z3/Z3GateScheduleConfigurator.h"

namespace inet {

Define_Module(Z3GateScheduleConfigurator);

Z3GateScheduleConfigurator::~Z3GateScheduleConfigurator()
{
    delete z3Solver;
    delete z3Optimize;
    variables.clear();
    delete z3Context;
}

void Z3GateScheduleConfigurator::initialize(int stage)
{
    GateScheduleConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        labelAsserts = par("labelAsserts");
        optimizeSchedule = par("optimizeSchedule");
    }
}

void Z3GateScheduleConfigurator::addAssert(const expr& expr) const
{
    static int assertionCount = 0;
    if (optimizeSchedule) {
        if (labelAsserts)
            z3Optimize->add(expr, (std::string("a") + std::to_string(assertionCount++)).c_str());
        else
            z3Optimize->add(expr);
    }
    else {
        if (labelAsserts)
            z3Solver->add(expr, (std::string("a") + std::to_string(assertionCount++)).c_str());
        else
            z3Solver->add(expr);
    }
}

Z3GateScheduleConfigurator::Output *Z3GateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    config cfg;
    cfg.set("model", "true");
    z3Context = new context(cfg);
    z3Solver = !optimizeSchedule ? new z3::solver(*z3Context) : nullptr;
    z3Optimize = optimizeSchedule ? new optimize(*z3Context) : nullptr;
    auto gateCycleDurationVariable = getGateCycleDurationVariable();
    addAssert(gateCycleDurationVariable == z3Context->real_val(gateCycleDuration.str().c_str()));
    auto totalEndToEndDelayVariable = getTotalEndToEndDelayVariable();
    auto totalEndToEndDelayValue = z3Context->real_val(0);

    // 1. add application start time constraints
    for (auto application : input.applications) {
        auto applicationStartTimeVariable = getApplicationStartTimeVariable(application);
        addAssert(expr(z3Context->real_val(0)) <= applicationStartTimeVariable);
        addAssert(applicationStartTimeVariable < gateCycleDurationVariable);
    }

    // 2. add transmission start/end time, reception start/end time, transmission duration, propagation time constraints
    for (auto flow : input.flows) {
        for (int packetIndex = 0; packetIndex < getPacketCount(flow); packetIndex++) {
            for (auto pathFragment : flow->pathFragments) {
                for (int nodeIndex = 0; nodeIndex < pathFragment->networkNodes.size() - 1; nodeIndex++) {
                    auto transmissionPort = pathFragment->outputPorts[nodeIndex];
                    auto transmissionDurationVariable = getTransmissionDurationVariable(flow, transmissionPort);
                    simtime_t transmissionDuration = s(flow->startApplication->packetLength / transmissionPort->datarate).get();
                    addAssert(transmissionDurationVariable == z3Context->real_val(transmissionDuration.str().c_str()));
                    auto transmissionStartTimeVariable = getTransmissionStartTimeVariable(flow, packetIndex, transmissionPort, flow->gateIndex);
                    auto transmissionEndTimeVariable = getTransmissionEndTimeVariable(flow, packetIndex, transmissionPort, flow->gateIndex);
                    addAssert(transmissionStartTimeVariable + transmissionDurationVariable == transmissionEndTimeVariable);
                    addAssert(expr(z3Context->real_val(0)) <= transmissionStartTimeVariable);
                    addAssert(transmissionStartTimeVariable < gateCycleDurationVariable);
                    auto receptionPort = pathFragment->inputPorts[nodeIndex];
                    auto receptionStartTimeVariable = getReceptionStartTimeVariable(flow, packetIndex, receptionPort, flow->gateIndex);
                    auto receptionEndTimeVariable = getReceptionEndTimeVariable(flow, packetIndex, receptionPort, flow->gateIndex);
                    addAssert(receptionStartTimeVariable + transmissionDurationVariable == receptionEndTimeVariable);
                    auto propagationTimeVariable = getPropagationTimeVariable(transmissionPort);
                    addAssert(propagationTimeVariable == z3Context->real_val(transmissionPort->propagationTime.str().c_str()));
                    addAssert(transmissionStartTimeVariable + propagationTimeVariable == receptionStartTimeVariable);
                }
            }
        }
    }

    // 3. add chained reception end time, transmission start time and end-to-end delay constraints
    for (auto flow : input.flows) {
        auto maxEndToEndDelayVariable = flow->startApplication->maxLatency > 0 ? getMaxEndToEndDelayVariable(flow) : nullptr;
        auto applicationStartTimeVariable = getApplicationStartTimeVariable(flow->startApplication);
        auto packetIntervalVariable = getApplicationPacketIntervalVariable(flow->startApplication);
        auto packetInterval = flow->startApplication->packetInterval;
        addAssert(packetIntervalVariable == z3Context->real_val(packetInterval.str().c_str()));
        for (int packetIndex = 0; packetIndex < getPacketCount(flow); packetIndex++) {
            std::shared_ptr<expr> firstTransmissionStartTimeVariable;
            std::shared_ptr<expr> previousReceptionEndTimeVariable;
            std::shared_ptr<expr> previousReceptionStartTimeVariable;
            for (auto pathFragment : flow->pathFragments) {
                for (int nodeIndex = 0; nodeIndex < pathFragment->networkNodes.size() - 1; nodeIndex++) {
                    auto transmissionPort = pathFragment->outputPorts[nodeIndex];
                    auto transmissionStartTimeVariable = getTransmissionStartTimeVariable(flow, packetIndex, transmissionPort, flow->gateIndex);
                    if (firstTransmissionStartTimeVariable == nullptr)
                        firstTransmissionStartTimeVariable = transmissionStartTimeVariable;
                    auto receptionPort = pathFragment->inputPorts[nodeIndex];
                    auto receptionStartTimeVariable = getReceptionStartTimeVariable(flow, packetIndex, receptionPort, flow->gateIndex);
                    auto receptionEndTimeVariable = getReceptionEndTimeVariable(flow, packetIndex, receptionPort, flow->gateIndex);
                    if (nodeIndex == 0)
                        addAssert(applicationStartTimeVariable + packetIntervalVariable * z3Context->real_val(packetIndex) == transmissionStartTimeVariable);
                    else if (previousReceptionEndTimeVariable) {
                        if (transmissionPort->cutthroughSwitchingEnabled && receptionPort->cutthroughSwitchingEnabled && flow->cutthroughSwitchingHeaderSize != b(0)) {
                            simtime_t cutthroughDelay = s(flow->cutthroughSwitchingHeaderSize / receptionPort->datarate).get();
                            addAssert(transmissionStartTimeVariable >= previousReceptionStartTimeVariable + z3Context->real_val(cutthroughDelay.str().c_str()));
                        }
                        else
                            addAssert(transmissionStartTimeVariable >= previousReceptionEndTimeVariable);
                    }
                    previousReceptionStartTimeVariable = receptionStartTimeVariable;
                    previousReceptionEndTimeVariable = receptionEndTimeVariable;
                }
            }
            auto endToEndDelayVariable = getEndToEndDelayVariable(flow, packetIndex);
            addAssert(endToEndDelayVariable == previousReceptionEndTimeVariable - firstTransmissionStartTimeVariable);
            totalEndToEndDelayValue = totalEndToEndDelayValue + endToEndDelayVariable;
            if (maxEndToEndDelayVariable)
                addAssert(endToEndDelayVariable <= maxEndToEndDelayVariable);
        }
        if (maxEndToEndDelayVariable)
            addAssert(maxEndToEndDelayVariable == z3Context->real_val(flow->startApplication->maxLatency.str().c_str()));
    }
    addAssert(totalEndToEndDelayVariable == totalEndToEndDelayValue);

    // 4. add jitter constraints
    for (auto flow : input.flows) {
        int count = getPacketCount(flow);
        auto maxJitterVariable = getMaxJitterVariable(flow);
        auto averageEndToEndDelayVariable = getAverageEndToEndDelayVariable(flow);
        auto averageEndToEndDelayValue = z3Context->real_val(0);
        for (int packetIndex = 0; packetIndex < count; packetIndex++) {
            auto endToEndDelayVariable = getEndToEndDelayVariable(flow, packetIndex);
            averageEndToEndDelayValue = averageEndToEndDelayValue + endToEndDelayVariable;
        }
        addAssert(averageEndToEndDelayVariable == averageEndToEndDelayValue / z3Context->real_val(count));
        for (int packetIndex = 0; packetIndex < count; packetIndex++) {
            auto endToEndDelayVariable = getEndToEndDelayVariable(flow, packetIndex);
            addAssert(averageEndToEndDelayVariable - endToEndDelayVariable <= maxJitterVariable);
        }
        addAssert(maxJitterVariable == z3Context->real_val(0));
    }

    // 5. add one transmission per port at a time constraints
    for (auto port : input.ports) {
        simtime_t interframeGap = s(b(96) / port->datarate).get();
        auto interframeGapVariable = getInterframeGapVariable(port);
        addAssert(interframeGapVariable == z3Context->real_val(interframeGap.str().c_str()));
        auto transmissionStartTimeVariables = getTransmissionStartTimeVariables(port);
        auto transmissionEndTimeVariables = getTransmissionEndTimeVariables(port);
        for (int i = 0; i < transmissionStartTimeVariables.size(); i++) {
            auto transmissionStartTimeVariableI = transmissionStartTimeVariables[i];
            auto transmissionEndTimeVariableI = transmissionEndTimeVariables[i];
            for (int j = i + 1; j < transmissionStartTimeVariables.size(); j++) {
                auto transmissionStartTimeVariableJ = transmissionStartTimeVariables[j];
                auto transmissionEndTimeVariableJ = transmissionEndTimeVariables[j];
                addAssert(transmissionEndTimeVariableI + interframeGapVariable <= transmissionStartTimeVariableJ ||
                          transmissionEndTimeVariableJ + interframeGapVariable <= transmissionStartTimeVariableI);
            }
        }
    }

    // 6. add queueing constraints to prevent reordering packets
    for (auto port : input.ports) {
        for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
            auto transmissionEndTimeVariables = getTransmissionEndTimeVariables(port, gateIndex);
            auto receptionEndTimeVariables = getReceptionEndTimeVariables(port, gateIndex);
            for (int i = 0; i < transmissionEndTimeVariables.size(); i++) {
                auto transmissionEndTimeVariableI = transmissionEndTimeVariables[i];
                if (i < receptionEndTimeVariables.size()) {
                    auto receptionEndTimeVariableI = receptionEndTimeVariables[i];
                    for (int j = i + 1; j < transmissionEndTimeVariables.size(); j++) {
                        auto transmissionEndTimeVariableJ = transmissionEndTimeVariables[j];
                        if (j < receptionEndTimeVariables.size()) {
                            auto receptionEndTimeVariableJ = receptionEndTimeVariables[j];
                            addAssert(receptionEndTimeVariableI < receptionEndTimeVariableJ == transmissionEndTimeVariableI < transmissionEndTimeVariableJ);
                        }
                    }
                }
            }
        }
    }

    // 7. add traffic class constraints to prevent lower priority to transmit first
    for (auto port : input.ports) {
        for (int gateIndexI = 0; gateIndexI < port->numGates; gateIndexI++) {
            auto transmissionStartTimeVariablesI = getTransmissionStartTimeVariables(port, gateIndexI);
            auto receptionEndTimeVariablesI = getReceptionEndTimeVariables(port, gateIndexI);
            for (int gateIndexJ = gateIndexI + 1; gateIndexJ < port->numGates; gateIndexJ++) {
                auto transmissionStartTimeVariablesJ = getTransmissionStartTimeVariables(port, gateIndexJ);
                for (int i = 0; i < receptionEndTimeVariablesI.size(); i++) {
                    auto receptionEndTimeVariableI = receptionEndTimeVariablesI[i];
                    if (i < transmissionStartTimeVariablesI.size()) {
                        auto transmissionStartTimeVariableI = transmissionStartTimeVariablesI[i];
                        for (int j = 0; j < transmissionStartTimeVariablesJ.size(); j++) {
                            auto transmissionStartTimeVariableJ = transmissionStartTimeVariablesJ[j];
                            addAssert(transmissionStartTimeVariableJ <= receptionEndTimeVariableI || transmissionStartTimeVariableJ >= transmissionStartTimeVariableI);
                        }
                    }
                }
            }
        }
    }

    // 8. add minimization goal if requested
    if (optimizeSchedule)
        // TODO add weights for the end-to-end delay and jitter of individual flows
        z3Optimize->minimize(*totalEndToEndDelayVariable.get());

    // print what we have
    EV_INFO << "Goal:" << std::endl;
    if (optimizeSchedule)
        EV_INFO << *z3Optimize << std::endl;
    else
        EV_INFO << *z3Solver << std::endl;

    // solve
    if ((optimizeSchedule ? z3Optimize->check() : z3Solver->check()) == sat) {
        model model = optimizeSchedule ? z3Optimize->get_model() : z3Solver->get_model();
        EV_INFO << "Solution:" << std::endl << model << std::endl;
        auto output = new Output();

        // 1. fill application start times
        for (auto application : input.applications)
            output->applicationStartTimes[application] = getVariableValue(model, getApplicationStartTimeVariable(application));

        // 2. print some detailed results
        for (auto flow : input.flows) {
            for (int packetIndex = 0; packetIndex < getPacketCount(flow); packetIndex++) {
                for (auto pathFragment : flow->pathFragments) {
                    for (int nodeIndex = 0; nodeIndex < pathFragment->networkNodes.size() - 1; nodeIndex++) {
                        auto transmissionPort = pathFragment->outputPorts[nodeIndex];
                        double transmissionStartTime = getVariableValue(model, getTransmissionStartTimeVariable(flow, packetIndex, transmissionPort, flow->gateIndex));
                        double transmissionEndTime = getVariableValue(model, getTransmissionEndTimeVariable(flow, packetIndex, transmissionPort, flow->gateIndex));
                        EV_DEBUG << "Transmission: " << flow->name << ".packet" << std::to_string(packetIndex) << transmissionPort->module->getFullPath() << ", start = " << transmissionStartTime << ", end = " << transmissionEndTime << std::endl;
                    }
                }
                EV_DEBUG << "End-to-end delay: " << getVariableValue(model, getEndToEndDelayVariable(flow, packetIndex)) << std::endl;
            }
        }

        // 3. fill gate schedules
        for (auto port : input.ports) {
            auto& schedules = output->gateSchedules[port];
            for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
                EV_DEBUG << "Computing schedule, port = " << port->module->getFullPath() << ", gateIndex = " << gateIndex << std::endl;
                auto schedule = new Output::Schedule();
                schedule->port = port;
                schedule->gateIndex = gateIndex;
                schedule->cycleStart = 0;
                schedule->cycleDuration = gateCycleDuration;
                auto transmissionStartTimeVariables = getTransmissionStartTimeVariables(port, gateIndex);
                auto transmissionEndTimeVariables = getTransmissionEndTimeVariables(port, gateIndex);
                for (int i = 0; i < transmissionStartTimeVariables.size(); i++) {
                    simtime_t slotStart = getVariableValue(model, transmissionStartTimeVariables[i]);
                    simtime_t slotEnd = getVariableValue(model, transmissionEndTimeVariables[i]);
                    simtime_t slotDuration = slotEnd - slotStart;
                    EV_DEBUG << "Adding slot, start time = " << slotStart << ", end time = " << slotEnd << std::endl;
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
                        scheduleSlot.start = SimTime::fromRaw(slotStart.raw() % gateCycleDuration.raw());
                        scheduleSlot.duration = slotDuration;
                        schedule->slots.push_back(scheduleSlot);
                    }
                }
                auto& slots = schedule->slots;
                std::sort(slots.begin(), slots.end(), [] (const Output::Slot& slot1, const Output::Slot& slot2) {
                    return slot1.start < slot2.start;
                });
                for (auto slot : slots)
                    EV_DEBUG << "Added slot, start time = " << slot.start << ", duration = " << slot.duration << std::endl;
                schedules.push_back(schedule);
            }
        }

        return output;
    }
    else {
        EV_WARN << "No solution found, unsatisfiable core:" << std::endl << (optimizeSchedule ? z3Solver->unsat_core() : z3Optimize->unsat_core()) << std::endl;
        throw cRuntimeError("The specified constraints might not be satisfiable.");
    }
}

double Z3GateScheduleConfigurator::getVariableValue(const model& model, const std::shared_ptr<expr> expr) const
{
    auto str = model.eval(*expr.get(), false).to_string();
    int index = str.find('/');
    if (index != std::string::npos) {
        str = str.substr(3, str.length() - 1);
        index = str.find(' ');
        double val1 = atof(str.substr(0, index).c_str());
        double val2 = atof(str.substr(index + 1).c_str());
        return val1 / val2;
    }
    else
        return atof(str.c_str());
}

} // namespace inet

