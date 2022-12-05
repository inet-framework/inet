//
// Copyright (C) 2021 OpenSim Ltd. and the original authors
//
// This file is partly copied from the following project with the explicit
// permission from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_Z3GATESCHEDULECONFIGURATOR_H
#define __INET_Z3GATESCHEDULECONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

#include "inet/linklayer/configurator/gatescheduling/z3/Operator.h"

namespace inet {

using namespace z3;

class INET_API Z3GateScheduleConfigurator : public GateScheduleConfiguratorBase
{
  protected:
    bool labelAsserts = false;
    bool optimizeSchedule = true;
    mutable context *z3Context = nullptr;
    mutable solver *z3Solver = nullptr;
    mutable optimize *z3Optimize = nullptr;
    mutable std::map<std::string, std::shared_ptr<expr>> variables;

  protected:
    virtual void initialize(int stage) override;

    virtual Output *computeGateScheduling(const Input& input) const override;

    virtual void addAssert(const expr& expr) const;

    virtual std::shared_ptr<expr> getVariable(std::string variableName) const {
        auto it = variables.find(variableName);
        if (it != variables.end())
            return it->second;
        else {
            auto variable = std::make_shared<expr>(z3Context->real_const(variableName.c_str()));
            variables[variableName] = variable;
            return variable;
        }
    }

    virtual std::string getFullPath(cObject *object) const {
        std::string fullPath = object->getFullPath();
        return fullPath.substr(fullPath.find('.') + 1);
    }

    virtual int getPacketCount(const Input::Flow *flow) const {
        auto packetInterval = flow->startApplication->packetInterval;
        int count = gateCycleDuration / packetInterval;
        if (gateCycleDuration != count * packetInterval)
            throw cRuntimeError("Gate cycle duration must be a multiple of the application packet interval");
        return count;
    }

    virtual std::shared_ptr<expr> getGateCycleDurationVariable() const {
        return getVariable("gateCycleDuration");
    }

    virtual std::shared_ptr<expr> getInterframeGapVariable(const Input::Port* port) const {
        std::string variableName = getFullPath(port->module) + ".interframeGap";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getApplicationStartTimeVariable(const Input::Application *application) const {
        std::string variableName = getFullPath(application->module) + ".startTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getApplicationPacketIntervalVariable(const Input::Application *application) const {
        std::string variableName = getFullPath(application->module) + ".packetInterval";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getPropagationTimeVariable(const Input::Port *port) const {
        std::string variableName = getFullPath(port->module) + ".propagationTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getTransmissionDurationVariable(const Input::Flow *flow, const Input::Port *port) const {
        std::string variableName = flow->name + "." + getFullPath(port->module) + ".transmissionDuration";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getTransmissionStartTimeVariable(const Input::Flow *flow, int packetIndex, const Input::Port *port, int gateIndex) const {
        std::string variableName = flow->name + ".packet" + std::to_string(packetIndex) + "." + getFullPath(port->module) + ".gate" + std::to_string(gateIndex) + ".transmissionStartTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getTransmissionEndTimeVariable(const Input::Flow *flow, int packetIndex, const Input::Port *port, int gateIndex) const {
        std::string variableName = flow->name + ".packet" + std::to_string(packetIndex) + "." + getFullPath(port->module) + ".gate" + std::to_string(gateIndex) + ".transmissionEndTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getReceptionStartTimeVariable(const Input::Flow *flow, int packetIndex, const Input::Port *port, int gateIndex) const {
        std::string variableName = flow->name + ".packet" + std::to_string(packetIndex) + "." + getFullPath(port->module) + ".gate" + std::to_string(gateIndex) + ".receptionStartTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getReceptionEndTimeVariable(const Input::Flow *flow, int packetIndex, const Input::Port *port, int gateIndex) const {
        std::string variableName = flow->name + ".packet" + std::to_string(packetIndex) + "." + getFullPath(port->module) + ".gate" + std::to_string(gateIndex) + ".receptionEndTime";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getMaxEndToEndDelayVariable(const Input::Flow *flow) const {
        std::string variableName = flow->name + ".maxEndToEndDelay";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getMaxJitterVariable(const Input::Flow *flow) const {
        std::string variableName = flow->name + ".maxJitter";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getEndToEndDelayVariable(const Input::Flow *flow, int packetIndex) const {
        std::string variableName = flow->name + ".packet" + std::to_string(packetIndex) + ".endToEndDelay";
        return getVariable(variableName);
    }

    virtual std::shared_ptr<expr> getTotalEndToEndDelayVariable() const {
        return getVariable("totalEndToEndDelay");
    }

    virtual std::shared_ptr<expr> getAverageEndToEndDelayVariable(const Input::Flow *flow) const {
        std::string variableName = flow->name + ".averageEndToEndDelay";
        return getVariable(variableName);
    }

    virtual std::vector<std::shared_ptr<expr>> getTimeVariables(const Input::Port *port, int gateIndex, std::string suffix) const {
        std::vector<std::shared_ptr<expr>> result;
        for (auto it : variables) {
            auto variableName = it.first;
            if (variableName.find(suffix) != std::string::npos &&
                variableName.find(getFullPath(port->module)) != std::string::npos &&
                (gateIndex == -1 || variableName.find(std::string("gate") + std::to_string(gateIndex)) != std::string::npos))
            {
                result.push_back(it.second);
            }
        }
        std::sort(result.begin(), result.end(), [&] (const auto& expr1, const auto& expr2) { return expr1->to_string() < expr2->to_string(); });
        return result;
    }

    virtual std::vector<std::shared_ptr<expr>> getTransmissionStartTimeVariables(const Input::Port *port, int gateIndex = -1) const {
        return getTimeVariables(port, gateIndex, "transmissionStartTime");
    }

    virtual std::vector<std::shared_ptr<expr>> getTransmissionEndTimeVariables(const Input::Port *port, int gateIndex = -1) const {
        return getTimeVariables(port, gateIndex, "transmissionEndTime");
    }

    virtual std::vector<std::shared_ptr<expr>> getReceptionStartTimeVariables(const Input::Port *port, int gateIndex = -1) const {
        return getTimeVariables(port, gateIndex, "receptionStartTime");
    }

    virtual std::vector<std::shared_ptr<expr>> getReceptionEndTimeVariables(const Input::Port *port, int gateIndex = -1) const {
        return getTimeVariables(port, gateIndex, "receptionEndTime");
    }

    virtual double getVariableValue(const model& model, const std::shared_ptr<expr> expr) const;

  public:
    virtual ~Z3GateScheduleConfigurator();
};

} // namespace inet

#endif

