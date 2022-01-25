//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_DEVICE_H
#define __INET_DEVICE_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Defs.h"

namespace inet {

using namespace z3;

/**
 * Specifies properties of device nodes in the network.
 * They can be used as sending devices and receiving devices.
 * Their properties specify part of the core of a flow.
 */
class INET_API Device : public cObject {
  public:
    std::string name;
    double packetPeriodicity;
    double firstT1Time;
    double hardConstraintTime;
    double packetSize;

    static int indexCounter;
    std::shared_ptr<expr> packetPeriodicityZ3;
    std::shared_ptr<expr> firstT1TimeZ3;
    std::shared_ptr<expr> hardConstraintTimeZ3;
    std::shared_ptr<expr> packetSizeZ3;
    std::shared_ptr<expr> flowPriority;

    /**
     * Default constructor method of a device.
     * Sets the newly created device name.
     */
    Device() {
        this->name = std::string("dev") + std::to_string(indexCounter++);
    }

    /**
     * Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * double values. These values will later be converted tostd::to_string(
     * z3 values. Used for simplified configurations. Other
     * constructors either are deprecated or set parameters
     * that will be used in future works.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param hardConstraintTime    Maximum latency tolerated by this device
     */
    Device(double packetPeriodicity,
           double hardConstraintTime) {
        this->packetPeriodicity = packetPeriodicity;
        this->firstT1Time = 0;
        this->hardConstraintTime = hardConstraintTime;
        this->packetSize = 0;
        this->name = std::string("dev") + std::to_string(++indexCounter);
    }

    /**
     * Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * double values. These values will later be converted to
     * z3 values.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param firstT1Time           Time where the first packet is sent
     * @param hardConstraintTime    Maximum latency tolerated by this device
     * @param packetSize            Size of the packets sent by this device
     */
    Device(double packetPeriodicity,
           double firstT1Time,
           double hardConstraintTime,
           double packetSize) {
        this->packetPeriodicity = packetPeriodicity;
        this->firstT1Time = firstT1Time;
        this->hardConstraintTime = hardConstraintTime;
        this->packetSize = packetSize;
        this->name = std::string("dev") + std::to_string(++indexCounter);
    }

    /**
     * Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * z3 values.
     *
     * @param packetPeriodicityZ3       Periodicity of packet sending
     * @param firstT1TimeZ3             Time where the first packet is sent
     * @param hardConstraintTimeZ3      Maximum latency tolerated by this device
     * @param packetSizeZ3              Size of the packets sent by this device
     * @param flowPriority              Defines the priority queue in which this device packets belongs to (Not used yet)
     */
     Device(std::shared_ptr<expr> packetPeriodicityZ3,
            std::shared_ptr<expr> firstT1TimeZ3,
            std::shared_ptr<expr> hardConstraintTimeZ3,
            std::shared_ptr<expr> packetSizeZ3,
            std::shared_ptr<expr> flowPriority) {
        this->packetPeriodicityZ3 = packetPeriodicityZ3;
        this->firstT1TimeZ3 = firstT1TimeZ3;
        this->hardConstraintTimeZ3 = hardConstraintTimeZ3;
        this->packetSizeZ3 = packetSizeZ3;
        this->flowPriority = flowPriority;
        this->name = std::string("dev") + std::to_string(indexCounter++);
    }

    /**
     * After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
     void toZ3(context& ctx) {
        this->packetPeriodicityZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(packetPeriodicity).c_str()));
        //firstT1TimeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(firstT1Time))); // In case of fixed firstT1Time
        this->firstT1TimeZ3 = std::make_shared<expr>(ctx.real_const((name + std::string("FirstT1Time")).c_str()));
        this->hardConstraintTimeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(hardConstraintTime).c_str()));
        this->packetSizeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(packetSize).c_str()));
    }

    std::string getName() {
        return name;
    }

    void setName(std::string name) {
        this->name = name;
    }

    double getPacketPeriodicity() {
        return packetPeriodicity;
    }

    void setPacketPeriodicity(double packetPeriodicity) {
        this->packetPeriodicity = packetPeriodicity;
    }

    double getFirstT1Time() {
        return firstT1Time;
    }

    void setFirstT1Time(double firstT1Time) {
        this->firstT1Time = firstT1Time;
    }

    double getHardConstraintTime() {
        return hardConstraintTime;
    }

    void setHardConstraintTime(double hardConstraintTime) {
        this->hardConstraintTime = hardConstraintTime;
    }

    double getPacketSize() {
        return packetSize;
    }

    void setPacketSize(double packetSize) {
        this->packetSize = packetSize;
    }

    std::shared_ptr<expr> getPacketPeriodicityZ3() {
        return packetPeriodicityZ3;
    }

    void setPacketPeriodicityZ3(std::shared_ptr<expr> packetPeriodicity) {
        this->packetPeriodicityZ3 = packetPeriodicity;
    }

    std::shared_ptr<expr> getFirstT1TimeZ3() {
        return firstT1TimeZ3;
    }

    void setFirstT1TimeZ3(std::shared_ptr<expr> firstT1Time) {
        this->firstT1TimeZ3 = firstT1Time;
    }

    std::shared_ptr<expr> getHardConstraintTimeZ3() {
        return hardConstraintTimeZ3;
    }

    void setHardConstraintTimeZ3(std::shared_ptr<expr> hardConstraintTime) {
        this->hardConstraintTimeZ3 = hardConstraintTime;
    }

    std::shared_ptr<expr> getPacketSizeZ3() {
        return packetSizeZ3;
    }

    void setPacketSizeZ3(std::shared_ptr<expr> packetSize) {
        this->packetSizeZ3 = packetSize;
    }

    std::shared_ptr<expr> getFlowPriority() {
        return flowPriority;
    }

    void setFlowPriority(std::shared_ptr<expr> flowPriority) {
        this->flowPriority = flowPriority;
    }
};

}

#endif

