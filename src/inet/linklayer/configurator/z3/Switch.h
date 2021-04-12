#ifndef __INET_Z3_SWITCH_H
#define __INET_Z3_SWITCH_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * [Class]: Switch
 * [Usage]: Contains most of the properties of a normal
 * switch that are used to build the schedule. Since this
 * scheduler doesn't take in consideration scenarios where
 * normal switches and TSN switches interact, no z3 properties
 * had to be specified in this class.
 *
 * Can be used to further extend the usability of this
 * project in the future.
 *
 */
class INET_API Switch {

	protected std::string name;
    protected float maxPacketSize;
    protected float timeToTravel;
    protected float transmissionTime;
    protected float portSpeed;

    //protected z3::expr maxPacketSizeZ3;
    //protected z3::expr timeToTravelZ3;
    //protected z3::expr transmissionTimeZ3;
    //protected z3::expr portSpeedZ3;


    /**
     * [Method]: Switch
     * [Usage]: Default constructor of class Switch. Is used by
     * TSNSwitch to instantiate new children classes.
     */
    Switch() {

    }

    /**
     * [Method]: Switch
     * [Usage]: Overloaded constructor of class Switch. Can
     * instantiate new switches using properties given as
     * parameters.
     *
     * @param name                  Name of the switch
     * @param maxPacketSize         Maximum size of a packet supported by this switch
     * @param timeToTravel          Time that a packet takes to leave its port and reach the destination
     * @param transmissionTime      Time taken to process the packet inside the switch
     * @param portSpeed             Transmission speed of the port
     */
    Switch(std::string name,
                  float maxPacketSize,
                  float timeToTravel,
                  float transmissionTime,
                  float portSpeed) {
        this->name = name;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->transmissionTime = transmissionTime;
        this->portSpeed = portSpeed;
    }


    /*
     *  GETTERS AND SETTERS
     */


    float getMaxPacketSize() {
        return maxPacketSize;
    }

    void setMaxPacketSize(float maxPacketSize) {
        this->maxPacketSize = maxPacketSize;
    }

    float getTimeToTravel() {
        return timeToTravel;
    }

    void setTimeToTravel(float timeToTravel) {
        this->timeToTravel = timeToTravel;
    }

    float getTransmissionTime() {
        return transmissionTime;
    }

    void setTransmissionTime(float transmissionTime) {
        this->transmissionTime = transmissionTime;
    }

    float getPortSpeed() {
        return portSpeed;
    }

    void setPortSpeed(float portSpeed) {
        this->portSpeed = portSpeed;
    }

    void setName(std::string name) {
        this->name = name;
    }

    std::string getName() {
        return this->name;
    }
};

}

#endif

