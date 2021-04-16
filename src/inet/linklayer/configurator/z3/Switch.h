#ifndef __INET_Z3_SWITCH_H
#define __INET_Z3_SWITCH_H

#include <z3++.h>

#include "inet/linklayer/configurator/z3/Defs.h"

namespace inet {

using namespace z3;

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
class INET_API Switch : public cObject {
  public:
    std::string name;
    double maxPacketSize;
    double timeToTravel;
    double transmissionTime;
    double portSpeed;

    //z3::expr maxPacketSizeZ3;
    //z3::expr timeToTravelZ3;
    //z3::expr transmissionTimeZ3;
    //z3::expr portSpeedZ3;


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
           double maxPacketSize,
           double timeToTravel,
           double transmissionTime,
           double portSpeed) {
        this->name = name;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->transmissionTime = transmissionTime;
        this->portSpeed = portSpeed;
    }


    /*
     *  GETTERS AND SETTERS
     */


    double getMaxPacketSize() {
        return maxPacketSize;
    }

    void setMaxPacketSize(double maxPacketSize) {
        this->maxPacketSize = maxPacketSize;
    }

    double getTimeToTravel() {
        return timeToTravel;
    }

    void setTimeToTravel(double timeToTravel) {
        this->timeToTravel = timeToTravel;
    }

    double getTransmissionTime() {
        return transmissionTime;
    }

    void setTransmissionTime(double transmissionTime) {
        this->transmissionTime = transmissionTime;
    }

    double getPortSpeed() {
        return portSpeed;
    }

    void setPortSpeed(double portSpeed) {
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

