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

#include "inet/linklayer/configurator/gatescheduling/TSNschedGateSchedulingConfigurator.h"

#include <fstream>

namespace inet {

Define_Module(TSNschedGateSchedulingConfigurator);

static void printJson(std::ostream& stream, const cValue& value, int level = 0)
{
    std::string indent(level * 2, ' ');
    if (value.getType() == cValue::OBJECT) {
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

cValueMap *TSNschedGateSchedulingConfigurator::convertInputToJson(const Input& input) const
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
        // KLUDGE TODO use per port parameters
        double datarate = 1E+9 / 1000000;
        double propagationTime = 50E-9 * 1000000;
        // TODO KLUDGE this is a wild guess
        double guardBand = 0;
        for (auto flow : input.flows) {
            double v = b(flow->startApplication->packetLength).get() / datarate;
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
            jsonPort->set("name", port->module->getFullName());
            jsonPort->set("connectsTo", port->endNode->module->getFullName());
            jsonPort->set("timeToTravel", propagationTime);
            jsonPort->set("timeToTravelUnit", "us");
            jsonPort->set("guardBandSize", guardBand);
            jsonPort->set("guardBandSizeUnit", "us");
            jsonPort->set("portSpeed", datarate);
            jsonPort->set("portSpeedSizeUnit", "b");
            jsonPort->set("portSpeedTimeUnit", "us");
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
        jsonFlow->set("priority", flow->startApplication->priority);
        jsonFlow->set("packetPeriodicity", flow->startApplication->packetInterval.dbl() * 1000000);
        jsonFlow->set("packetPeriodicityUnit", "us");
        jsonFlow->set("packetSize", b(flow->startApplication->packetLength + B(12)).get());
        jsonFlow->set("packetSizeUnit", "b");
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

TSNschedGateSchedulingConfigurator::Output *TSNschedGateSchedulingConfigurator::convertJsonToOutput(const Input& input, const cValueMap *json) const
{
    auto output = new Output();
    auto jsonSwitches = check_and_cast<cValueArray *>(json->get("switches").objectValue());
    for (int i = 0; i < jsonSwitches->size(); i++) {
        auto jsonSwitch = check_and_cast<cValueMap *>(jsonSwitches->get(i).objectValue());
        std::string switchName = jsonSwitch->get("name").stringValue();
        auto switch_ = *std::find_if(input.switches.begin(), input.switches.end(), [&] (Input::Switch *switch_) { return switch_->module->getFullName() == switchName; });
        auto jsonPorts = check_and_cast<cValueArray *>(jsonSwitch->get("ports").objectValue());
        for (int j = 0; j < jsonPorts->size(); j++) {
            auto jsonPort = check_and_cast<cValueMap *>(jsonPorts->get(j).objectValue());
            std::string portName = jsonPort->get("name").stringValue();
            auto port = *std::find_if(switch_->ports.begin(), switch_->ports.end(), [&] (Input::Port *port) { return port->module->getFullName() == switchName; });
            auto& schedules = output->gateSchedules[port];
            for (int priority = 0; priority < port->numPriorities; priority++) {
                auto schedule = new Output::Schedule();
                schedule->port = port;
                schedule->priority = priority;
                auto jsonPrioritySlots = check_and_cast<cValueArray *>(jsonPort->get("prioritySlotData").objectValue());
                for (int k = 0; k < jsonPorts->size(); k++) {
                    auto jsonPrioritySlot = check_and_cast<cValueMap *>(jsonPrioritySlots->get(k).objectValue());
                    if (priority == jsonPrioritySlot->get("priority").intValue()) {
                        auto jsonSlotData = check_and_cast<cValueMap *>(jsonPrioritySlot->get("slotsData").objectValue());
                        simtime_t slotStart = jsonSlotData->get("slotStart").doubleValue();
                        simtime_t slotDuration = jsonSlotData->get("slotDuration").doubleValue();
                        // slot with length 0 are not used
                        if (slotDuration == 0)
                            continue;
                        Output::Slot slot;
                        slot.start = slotStart;
                        slot.duration = slotDuration;
                        schedule->slots.push_back(slot);
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
        auto name = jsonFlow->get("name").stringValue();
        auto flow = *std::find_if(input.flows.begin(), input.flows.end(), [&] (Input::Flow *flow) { return flow->name == name; });
        auto firstSendingTime = jsonFlow->get("firstSendingTime").doubleValue();
        output->applicationStartTimes[flow->startApplication] = firstSendingTime;
    }
    return output;
}

void TSNschedGateSchedulingConfigurator::writeInputToFile(const Input& input, std::string fileName) const
{
    auto json = convertInputToJson(input);
    std::ofstream stream;
    stream.open(fileName.c_str());
    printJson(stream, cValue(json));
    delete json;
}

TSNschedGateSchedulingConfigurator::Output *TSNschedGateSchedulingConfigurator::readOutputFromFile(const Input& input, std::string fileName) const
{
    std::string expression = std::string("readJSON(\"") + fileName + "\")";
    cDynamicExpression dynamicExression;
    dynamicExression.parse(expression.c_str());
    auto json = check_and_cast<cValueMap *>(dynamicExression.evaluate().objectValue());
    auto output = convertJsonToOutput(input, json);
    delete json;
    return output;
}

void TSNschedGateSchedulingConfigurator::executeTSNsched(std::string inputFileName) const
{
    std::string classpath = "${TSNSCHED_HOME}/libs/com.microsoft.z3.jar";
    std::string command = std::string("java -classpath ") + classpath + " -jar ${TSNSCHED_HOME}/libs/TSNsched.jar " + inputFileName;
    std::system(command.c_str());
}

TSNschedGateSchedulingConfigurator::Output *TSNschedGateSchedulingConfigurator::computeGateScheduling(const Input& input) const
{
    std::string inputFileName = "input.json";
    std::string outputFileName = "output.json";
    writeInputToFile(input, inputFileName);
    executeTSNsched(inputFileName);
    return readOutputFromFile(input, outputFileName);
}

} // namespace inet

