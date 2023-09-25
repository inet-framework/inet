//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/common/TSNschedGateScheduleConfigurator.h"

#include <cstdio>
#include <fstream>

namespace inet {

Define_Module(TSNschedGateScheduleConfigurator);

static void printJson(std::ostream& stream, const cValue& value, int level = 0)
{
    std::string indent(level * 2, ' ');
#if OMNETPP_BUILDNUM < 1527
    if (value.getType() == cValue::OBJECT) {
#else
    if (value.getType() == cNedValue::POINTER && value.containsObject()) {
#endif
        auto object = value.objectValue();
        if (auto array = dynamic_cast<cValueArray *>(object)) {
            if (array->size() == 0)
                stream << "[]";
            else {
                stream << "[\n";
                for (int i = 0; i < array->size(); i++) {
                    if (i != 0)
                        stream << ",\n";
                    stream << indent << "  ";
                    printJson(stream, array->get(i), level + 1);
                }
                stream << "\n" << indent << "]";
            }
        }
        else if (auto map = dynamic_cast<cValueMap *>(object)) {
            if (map->size() == 0)
                stream << "{}";
            else {
                stream << "{\n";
                auto it = map->getFields().begin();
                for (int i = 0; i < map->size(); i++, ++it) {
                    if (i != 0)
                        stream << ",\n";
                    stream << indent << "  " << it->first << ": ";
                    printJson(stream, it->second, level + 1);
                }
                stream << "\n" << indent << "}";
            }
        }
        else
            throw cRuntimeError("Unknown object type");
    }
    else
        stream << value.str();
}

cValueMap *TSNschedGateScheduleConfigurator::convertInputToJson(const Input& input) const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonDevices = new cValueArray();
    json->set("devices", jsonDevices);
    for (auto device : input.devices) {
        cValueMap *jsonDevice = new cValueMap();
        jsonDevices->add(jsonDevice);
        jsonDevice->set("name", device->module->getFullName());
    }
    cValueArray *jsonSwitches = new cValueArray();
    json->set("switches", jsonSwitches);
    for (int i = 0; i < input.switches.size(); i++) {
        auto switch_ = input.switches[i];
        cValueMap *jsonSwitch = new cValueMap();
        jsonSwitches->add(jsonSwitch);
        // TODO KLUDGE this is a wild guess
        double guardBand = 0;
        for (auto flow : input.flows) {
            double v = b(flow->startApplication->packetLength).get();
            if (guardBand < v)
                guardBand = v;
        }
        auto jsonPorts = new cValueArray();
        jsonSwitch->set("name", switch_->module->getFullName());
        jsonSwitch->set("ports", jsonPorts);
        for (int j = 0; j < switch_->ports.size(); j++) {
            auto port = switch_->ports[j];
            auto jsonPort = new cValueMap();
            jsonPorts->add(jsonPort);
            // KLUDGE: port name should not be unique in the network but only in the network node
            std::string nodeName = port->startNode->module->getFullName();
            jsonPort->set("name", nodeName + "-" + port->module->getFullName());
            jsonPort->set("connectsTo", port->endNode->module->getFullName());
            jsonPort->set("timeToTravel", port->propagationTime.dbl() * 1000000);
            jsonPort->set("timeToTravelUnit", "us");
//            jsonPort->set("guardBandSize", guardBand);
//            jsonPort->set("guardBandSizeUnit", "bit");
            jsonPort->set("portSpeed", bps(port->datarate).get() / 1000000);
            jsonPort->set("portSpeedSizeUnit", "bit");
            jsonPort->set("portSpeedTimeUnit", "us");
            jsonPort->set("scheduleType", "Hypercycle");
            jsonPort->set("cycleStart", 0);
            jsonPort->set("cycleStartUnit", "us");
            jsonPort->set("maximumSlotDuration", gateCycleDuration.dbl() * 1000000);
            jsonPort->set("maximumSlotDurationUnit", "us");
        }
    }
    cValueArray *jsonFlows = new cValueArray();
    json->set("flows", jsonFlows);
    for (auto flow : input.flows) {
        cValueMap *jsonFlow = new cValueMap();
        jsonFlows->add(jsonFlow);
        jsonFlow->set("name", flow->name);
        jsonFlow->set("type", "unicast");
        jsonFlow->set("sourceDevice", flow->startApplication->device->module->getFullName());
        jsonFlow->set("fixedPriority", "true");
        jsonFlow->set("priorityValue", flow->gateIndex);
        jsonFlow->set("packetPeriodicity", flow->startApplication->packetInterval.dbl() * 1000000);
        jsonFlow->set("packetPeriodicityUnit", "us");
        jsonFlow->set("packetSize", b(flow->startApplication->packetLength).get());
        jsonFlow->set("packetSizeUnit", "bit");
        jsonFlow->set("hardConstraintTime", flow->startApplication->maxLatency.dbl() * 1000000);
        jsonFlow->set("hardConstraintTimeUnit", "us");
        cValueArray *endDevices = new cValueArray();
        jsonFlow->set("endDevices", endDevices);
        endDevices->add(cValue(flow->endDevice->module->getFullName()));
        cValueArray *hops = new cValueArray();
        jsonFlow->set("hops", hops);
        for (int j = 0; j < flow->pathFragments.size(); j++) {
            auto pathFragment = flow->pathFragments[j];
            for (int k = 0; k < pathFragment->networkNodes.size() - 1; k++) {
                auto networkNode = pathFragment->networkNodes[k];
                auto nextNetworkNode = pathFragment->networkNodes[k + 1];
                cValueMap *hop = new cValueMap();
                hops->add(hop);
                hop->set("currentNodeName", networkNode->module->getFullName());
                hop->set("nextNodeName", nextNetworkNode->module->getFullName());
            }
        }
    }
    return json;
}

TSNschedGateScheduleConfigurator::Output *TSNschedGateScheduleConfigurator::convertJsonToOutput(const Input& input, const cValueMap *json) const
{
    auto output = new Output();
    auto jsonSwitches = check_and_cast<cValueArray *>(json->get("switches").objectValue());
    for (int i = 0; i < jsonSwitches->size(); i++) {
        auto jsonSwitch = check_and_cast<cValueMap *>(jsonSwitches->get(i).objectValue());
        std::string switchName = jsonSwitch->get("name").stringValue();
        auto it = std::find_if(input.switches.begin(), input.switches.end(), [&] (Input::Switch *switch_) { return switch_->module->getFullName() == switchName; });
        if (it == input.switches.end())
            throw cRuntimeError("Cannot find switch: %s", switchName.c_str());
        auto switch_ = *it;
        auto jsonPorts = check_and_cast<cValueArray *>(jsonSwitch->get("ports").objectValue());
        for (int j = 0; j < jsonPorts->size(); j++) {
            auto jsonPort = check_and_cast<cValueMap *>(jsonPorts->get(j).objectValue());
            std::string portName = jsonPort->get("name").stringValue();
            // KLUDGE: port name should not be unique in the network but only in the network node
            portName = portName.substr(portName.find('-') + 1);
            auto jt = std::find_if(switch_->ports.begin(), switch_->ports.end(), [&] (Input::Port *port) { return port->startNode == switch_ && port->module->getFullName() == portName; });
            if (jt == switch_->ports.end())
                throw cRuntimeError("Cannot find port: %s", portName.c_str());
            auto port = *jt;
            auto& schedules = output->gateSchedules[port];
            for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
                auto schedule = new Output::Schedule();
                schedule->port = port;
                schedule->gateIndex = gateIndex;
                schedule->cycleDuration = jsonPort->get("cycleDuration").doubleValue() / 1000000;
                auto jsonPrioritySlots = check_and_cast<cValueArray *>(jsonPort->get("prioritySlotsData").objectValue());
                for (int k = 0; k < jsonPrioritySlots->size(); k++) {
                    auto jsonPrioritySlot = check_and_cast<cValueMap *>(jsonPrioritySlots->get(k).objectValue());
                    if (gateIndex == jsonPrioritySlot->get("priority").intValue()) {
                        auto jsonSlots = check_and_cast<cValueArray *>(jsonPrioritySlot->get("slotsData").objectValue());
                        for (int l = 0; l < jsonSlots->size(); l++) {
                            auto jsonSlot = check_and_cast<cValueMap *>(jsonSlots->get(l).objectValue());
                            simtime_t slotStart = jsonSlot->get("slotStart").doubleValue() / 1000000;
                            simtime_t slotDuration = jsonSlot->get("slotDuration").doubleValue() / 1000000;
                            // slot with length 0 are not used
                            if (slotDuration == 0)
                                continue;
                            Output::Slot slot;
                            slot.start = slotStart;
                            slot.duration = slotDuration;
                            schedule->slots.push_back(slot);
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
    auto jsonFlows = check_and_cast<cValueArray *>(json->get("flows").objectValue());
    for (int i = 0; i < jsonFlows->size(); i++) {
        auto jsonFlow = check_and_cast<cValueMap *>(jsonFlows->get(i).objectValue());
        std::string name = jsonFlow->get("name").stringValue();
        auto it = std::find_if(input.flows.begin(), input.flows.end(), [&] (Input::Flow *flow) { return flow->name == name; });
        if (it == input.flows.end())
            throw cRuntimeError("Cannot find flow: %s", name.c_str());
        auto flow = *it;
        auto application = flow->startApplication;
        auto firstSendingTime = jsonFlow->get("firstSendingTime").doubleValue() / 1000000;
        bps datarate = application->device->ports[0]->datarate;
        auto startTime = firstSendingTime - s(application->packetLength / datarate).get();
        while (startTime < 0)
            startTime += application->packetInterval.dbl();
        output->applicationStartTimes[application] = startTime;
    }
    return output;
}

void TSNschedGateScheduleConfigurator::writeInputToFile(const Input& input, std::string fileName) const
{
    auto json = convertInputToJson(input);
    std::ofstream stream;
    stream.open(fileName.c_str());
    if (stream.fail())
        throw cRuntimeError("Cannot open file %s", fileName.c_str());
    printJson(stream, cValue(json));
    delete json;
}

TSNschedGateScheduleConfigurator::Output *TSNschedGateScheduleConfigurator::readOutputFromFile(const Input& input, std::string fileName) const
{
    std::ifstream stream(fileName.c_str());
    if (!stream.good())
        throw cRuntimeError("Cannot read from TSNsched output file");
    std::string expression = std::string("readJSON(\"") + fileName + "\")";
    cDynamicExpression dynamicExression;
    dynamicExression.parse(expression.c_str());
    auto json = check_and_cast<cValueMap *>(dynamicExression.evaluate().objectValue());
    auto output = convertJsonToOutput(input, json);
    delete json;
    return output;
}

void TSNschedGateScheduleConfigurator::executeTSNsched(std::string inputFileName) const
{
    std::string classpath = "${TSNSCHED_ROOT}/libs/com.microsoft.z3.jar";
    std::string command = std::string("java -classpath ") + classpath + " -jar ${TSNSCHED_ROOT}/libs/TSNsched.jar " + inputFileName + " -enableConsoleOutput";
    if (std::system(command.c_str()) != 0)
        throw cRuntimeError("TSNsched command execution failed, make sure TSNSCHED_ROOT is set and Microsoft Z3 is installed");
}

TSNschedGateScheduleConfigurator::Output *TSNschedGateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    std::string baseName = getEnvir()->getConfig()->substituteVariables("${resultdir}/${configname}-${iterationvarsf}#${repetition}");
    std::string inputFileName = baseName + "-TSNsched-input.json";
    // TODO: std::string outputFileName = baseName + "-TSNsched-output.json";
    std::string outputFileName = "output.json";
    std::remove(outputFileName.c_str());
    writeInputToFile(input, inputFileName);
    executeTSNsched(inputFileName);
    return readOutputFromFile(input, outputFileName);
}

} // namespace inet

