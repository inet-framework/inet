#ifndef __INET_Z3_DEVICE_H
#define __INET_Z3_DEVICE_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

// CLASS WHERE DEVICE PROPERTIES AND CONDITIONS ARE SPECIFIED

/**
 * [Class]: Device
 * [Usage]: Specifies properties of device nodes in the network.
 * They can be used as sending devices and receiving devices.
 * Their properties specify part of the core of a flow.
 */
class INET_API Device {

    std::string name;
    float packetPeriodicity;
    float firstT1Time;
    float hardConstraintTime;
    float softConstraintTime;
    float packetSize;

    static int indexCounter = -1;
    z3::expr packetPeriodicityZ3;
    z3::expr firstT1TimeZ3;
    z3::expr hardConstraintTimeZ3;
    z3::expr softConstraintTimeZ3;
    z3::expr packetSizeZ3;
    z3::expr flowPriority;

    /**
     * [Method]: Device
     * [Usage]: Default constructor method of a device.
     * Sets the newly created device name.
     */
    Device() {
		this->name = "dev" + indexCounter++;
	}

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values. Used for simplified configurations. Other
     * constructors either are deprecated or set parameters
     * that will be used in future works.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param hardConstraintTime    Maximum latency tolerated by this device
     */
    Device(float packetPeriodicity,
                  float hardConstraintTime) {
        this->packetPeriodicity = packetPeriodicity;
        this->firstT1Time = 0;
        this->hardConstraintTime = hardConstraintTime;
        this->softConstraintTime = 0;
        this->packetSize = 0;
        this->name = "dev" + ++indexCounter;
    }

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param firstT1Time           Time where the first packet is sent
     * @param hardConstraintTime    Maximum latency tolerated by this device
     * @param softConstraintTime    Recommended latency for using this device
     * @param packetSize            Size of the packets sent by this device
     */
    Device(float packetPeriodicity,
                  float firstT1Time,
                  float hardConstraintTime,
                  float softConstraintTime,
                  float packetSize) {
        this->packetPeriodicity = packetPeriodicity;
        this->firstT1Time = firstT1Time;
        this->hardConstraintTime = hardConstraintTime;
        this->softConstraintTime = softConstraintTime;
        this->packetSize = packetSize;
        this->name = "dev" + ++indexCounter;
    }

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param firstT1Time           Time where the first packet is sent
     * @param hardConstraintTime    Maximum latency tolerated by this device
     * @param packetSize            Size of the packets sent by this device
     */
    Device(float packetPeriodicity,
                  float firstT1Time,
                  float hardConstraintTime,
                  float packetSize) {
        this->packetPeriodicity = packetPeriodicity;
        this->firstT1Time = firstT1Time;
        this->hardConstraintTime = hardConstraintTime;
        this->softConstraintTime = 0;
        this->packetSize = packetSize;
        this->name = "dev" + ++indexCounter;
    }


    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * z3 values.
     *
     * @param packetPeriodicityZ3       Periodicity of packet sending
     * @param firstT1TimeZ3             Time where the first packet is sent
     * @param hardConstraintTimeZ3      Maximum latency tolerated by this device
     * @param softConstraintTimeZ3      Recommended latency for using this device
     * @param packetSizeZ3              Size of the packets sent by this device
     * @param flowPriority              Defines the priority queue in which this device packets belongs to (Not used yet)
     */
     Device(z3::expr packetPeriodicityZ3,
				  z3::expr firstT1TimeZ3,
				  z3::expr hardConstraintTimeZ3,
				  z3::expr softConstraintTimeZ3,
				  z3::expr packetSizeZ3,
				  z3::expr flowPriority) {
		this->packetPeriodicityZ3 = packetPeriodicityZ3;
		this->firstT1TimeZ3 = firstT1TimeZ3;
		this->hardConstraintTimeZ3 = hardConstraintTimeZ3;
		this->softConstraintTimeZ3 = softConstraintTimeZ3;
		this->packetSizeZ3 = packetSizeZ3;
		this->flowPriority = flowPriority;
        this->name = "dev" + indexCounter++;
	}

	/**
	 * [Method]: toZ3
	 * [Usage]: After setting all the numeric input values of the class,
	 * generates the z3 equivalent of these values and creates any extra
	 * variable needed.
	 *
	 * @param ctx      context variable containing the z3 environment used
	 */
     void toZ3(context ctx) {
	    this->packetPeriodicityZ3 = ctx.real_val(std::to_string(this->packetPeriodicity));
	    //this->firstT1TimeZ3 = ctx.real_val(std::to_string(this->firstT1Time)); // In case of fixed firstT1Time
	    this->firstT1TimeZ3 = ctx.real_const(this->name + "FirstT1Time");
	    this->hardConstraintTimeZ3 = ctx.real_val(std::to_string(this->hardConstraintTime));
	    this->softConstraintTimeZ3 = ctx.real_val(std::to_string(this->softConstraintTime));
	    this->packetSizeZ3 = ctx.real_val(std::to_string(this->packetSize));
	}

    /*
     *  GETTERS AND SETTERS
     */

    std::string getName() {
        return name;
    }

    void setName(std::string name) {
        this->name = name;
    }

     float getPacketPeriodicity() {
        return packetPeriodicity;
    }

    void setPacketPeriodicity(float packetPeriodicity) {
        this->packetPeriodicity = packetPeriodicity;
    }

    float getFirstT1Time() {
        return firstT1Time;
    }

    void setFirstT1Time(float firstT1Time) {
        this->firstT1Time = firstT1Time;
    }

    float getHardConstraintTime() {
        return hardConstraintTime;
    }

    void setHardConstraintTime(float hardConstraintTime) {
        this->hardConstraintTime = hardConstraintTime;
    }

    float getSoftConstraintTime() {
        return softConstraintTime;
    }

    void setSoftConstraintTime(float softConstraintTime) {
        this->softConstraintTime = softConstraintTime;
    }

    float getPacketSize() {
        return packetSize;
    }

    void setPacketSize(float packetSize) {
        this->packetSize = packetSize;
    }

    /*
     *  Z3 GETTERS AND SETTERS
     */

     z3::expr getPacketPeriodicityZ3() {
		return packetPeriodicityZ3;
	}

     void setPacketPeriodicityZ3(z3::expr packetPeriodicity) {
		this->packetPeriodicityZ3 = packetPeriodicity;
	}

     z3::expr getFirstT1TimeZ3() {
		return firstT1TimeZ3;
	}

     void setFirstT1TimeZ3(z3::expr firstT1Time) {
		this->firstT1TimeZ3 = firstT1Time;
	}

     z3::expr getHardConstraintTimeZ3() {
		return hardConstraintTimeZ3;
	}

     void setHardConstraintTimeZ3(z3::expr hardConstraintTime) {
		this->hardConstraintTimeZ3 = hardConstraintTime;
	}

     z3::expr getSoftConstraintTimeZ3() {
		return softConstraintTimeZ3;
	}

     void setSoftConstraintTimeZ3(z3::expr softConstraintTime) {
		this->softConstraintTimeZ3 = softConstraintTime;
	}

    z3::expr getPacketSizeZ3() {
        return packetSizeZ3;
    }

    void setPacketSizeZ3(z3::expr packetSize) {
        this->packetSizeZ3 = packetSize;
    }

    z3::expr getFlowPriority() {
        return flowPriority;
    }

    void setFlowPriority(z3::expr flowPriority) {
        this->flowPriority = flowPriority;
    }
};

}

#endif

