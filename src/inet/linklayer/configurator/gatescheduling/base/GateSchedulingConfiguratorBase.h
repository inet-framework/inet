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

#ifndef __INET_GATESCHEDULINGCONFIGURATORBASE_H
#define __INET_GATESCHEDULINGCONFIGURATORBASE_H

#include "inet/common/Units.h"
#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

using namespace units::values;

class INET_API GateSchedulingConfiguratorBase : public NetworkConfiguratorBase
{
  protected:
    class INET_API Input
    {
      public:
        class Port;

        class NetworkNode
        {
          public:
            std::vector<Port *> ports;
            cModule *module = nullptr;

          public:
            virtual ~NetworkNode() {}
        };

        class Device : public NetworkNode
        {
        };

        class Application
        {
          public:
            cModule *module = nullptr;
            int priority = -1;
            b packetLength = b(-1);
            simtime_t packetInterval = -1;
            simtime_t maxLatency = -1;
            Device *device = nullptr;
        };

        class Cycle
        {
          public:
            std::string name;
            simtime_t maxCycleTime = NaN;
            simtime_t maxSlotDuration = NaN;
        };

        class Port
        {
          public:
            cModule *module = nullptr;
            int numPriorities = -1;
            bps datarate = bps(NaN);
            simtime_t propagationTime = NaN;
            b maxPacketLength = b(-1);
            simtime_t guardBand = NaN;
            Cycle *cycle = nullptr;
            NetworkNode *startNode = nullptr;
            NetworkNode *endNode = nullptr;
        };

        class Switch : public NetworkNode
        {
        };

        class PathFragment
        {
          public:
            std::vector<NetworkNode *> networkNodes;
        };

        class Flow
        {
          public:
            std::string name;
            Application *startApplication = nullptr;
            Device *endDevice = nullptr;
            std::vector<PathFragment *> pathFragments;
        };

      public:
        std::vector<Device *> devices;
        std::vector<Application *> applications;
        std::vector<Cycle *> cycles;
        std::vector<Port *> ports;
        std::vector<Switch *> switches;
        std::vector<Flow *> flows;
        std::vector<NetworkNode *> networkNodes;

      public:
        ~Input() {
            for (auto device : devices) delete device;
            for (auto application : applications) delete application;
            for (auto cycle : cycles) delete cycle;
            for (auto port : ports) delete port;
            for (auto switch_ : switches) delete switch_;
            for (auto flow : flows) delete flow;
        }

        Device *findDevice(cModule *module) {
            auto it = std::find_if(devices.begin(), devices.end(), [&] (const Device *device) {
               return device->module == module;
            });
            return it != devices.end() ? *it : nullptr;
        }
        Device *getDevice(cModule *module) {
            auto device = findDevice(module);
            if (device == nullptr)
                throw cRuntimeError("Cannot find device");
            else
                return device;
        }

        Port *findPort(cModule *module) {
            auto it = std::find_if(ports.begin(), ports.end(), [&] (const Port *port) {
               return port->module == module;
            });
            return it != ports.end() ? *it : nullptr;
        }
        Port *getPort(cModule *module) {
            auto port = findPort(module);
            if (port == nullptr)
                throw cRuntimeError("Cannot find port");
            else
                return port;
        }

        NetworkNode *findNetworkNode(cModule *module) {
            auto it = std::find_if(networkNodes.begin(), networkNodes.end(), [&] (const NetworkNode *networkNode) {
               return networkNode->module == module;
            });
            return it != networkNodes.end() ? *it : nullptr;
        }
        NetworkNode *getNetworkNode(cModule *module) {
            auto networkNode = findNetworkNode(module);
            if (networkNode == nullptr)
                throw cRuntimeError("Cannot find network node");
            else
                return networkNode;
        }
    };

    class INET_API Output
    {
      public:
        class Schedule
        {
          public:
            Input::Port *port = nullptr;
            int priority = -1;
            simtime_t cycleStart = NaN;
            simtime_t cycleDuration = NaN;
            std::vector<simtime_t> slotStarts;
            std::vector<simtime_t> slotDurations;
        };

      public:
        std::map<Input::Port *, std::vector<Schedule *>> gateSchedules;
        std::map<Input::Application *, simtime_t> applicationStartTimes;

      public:
        ~Output() {
            for (auto it : gateSchedules)
                for (auto element : it.second)
                    delete element;
        }
    };

  protected:
    // parameters
    simtime_t gateCycleDuration;
    cValueArray *configuration = nullptr;

    // state
    Input *gateSchedulingInput = nullptr;
    Output *gateSchedulingOutput = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }

    virtual void clearConfiguration();
    virtual void computeConfiguration();

    virtual Input *createGateSchedulingInput() const;
    virtual void addDevices(Input& input) const;
    virtual void addSwitches(Input& input) const;
    virtual void addPorts(Input& input) const;
    virtual void addCycles(Input& input) const;
    virtual void addFlows(Input& input) const;

    virtual void configureGateScheduling();
    virtual void configureGateScheduling(cModule *networkNode, cModule *gate, InterfaceInfo *interfaceInfo);
    virtual void configureApplicationOffsets();

    virtual Output *computeGateScheduling(const Input& input) const = 0;

  public:
    virtual ~GateSchedulingConfiguratorBase() { clearConfiguration(); }
};

} // namespace inet

#endif

