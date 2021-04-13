#ifndef __INET_Z3_TSNSWITCH_H
#define __INET_Z3_TSNSWITCH_H

#include <z3++.h>

#include "inet/linklayer/configurator/z3/Device.h"
#include "inet/linklayer/configurator/z3/Port.h"
#include "inet/linklayer/configurator/z3/Switch.h"

namespace inet {

using namespace z3;

/**
 * [Class]: TSNSwitch
 * [Usage]: This class contains the information needed to
 * specify a switch capable of complying with the TSN patterns
 * to the schedule. Aside from part of the z3 data used to
 * generate the schedule, objects created from this class
 * are able to organize a sequence of ports that connect the
 * switch to other nodes in the network.
 */
class INET_API TSNSwitch : public Switch {
  public:
    bool isModifiedOrCreated = true;

    // private Cycle cycle;
    std::vector<std::string> connectsTo;
    std::vector<Port *> ports;
    float cycleDurationUpperBound;
    float cycleDurationLowerBound;

    float gbSize;
    std::shared_ptr<expr> gbSizeZ3; // Size of the guardBand
    std::shared_ptr<expr> cycleDuration;
    std::shared_ptr<expr> cycleStart;
    std::shared_ptr<expr> cycleDurationUpperBoundZ3;
    std::shared_ptr<expr> cycleDurationLowerBoundZ3;
    int portNum = 0;

    static int indexCounter;


    /**
     * [Method]: TSNSwitch
     * [Usage]: Overloaded constructor method of this class.
     * Creates a switch, giving it a name and creating a new
     * list of ports and labels of devices that it can reach.
     *
     * Can be adapted in the future to start the switch with
     * default properties.
     *
     * @param name      Name of the switch
     */
    TSNSwitch(std::string name) {
        this->name = name;
        ports.clear();
        this->connectsTo.clear();
    }

    /**
     * [Method]: TSNSwitch
     * [Usage]: Overloaded constructor method of this class.
     * Instantiates a new TSNSwitch object setting up its properties
     * that are given as parameters. Used for simplified configurations.
     * Other constructors either are deprecated or set parameters
     * that will be used in future works.
     *
     * @param timeToTravel          Time that a packet takes to leave its port and reach the destination
     * @param transmissionTime      Time taken to process the packet inside the switch
     */
    TSNSwitch(float timeToTravel,
                     float transmissionTime) {
        this->name = std::string("dev") + std::to_string(indexCounter++);
        this->timeToTravel = timeToTravel;
        this->transmissionTime = transmissionTime;
        this->ports.clear();
        this->connectsTo.clear();
        this->maxPacketSize = 0;
        this->portSpeed = 0;
        this->gbSize = 0;
    }


    /**
     * [Method]: TSNSwitch
     * [Usage]: Overloaded constructor method of this class.
     * Instantiates a new TSNSwitch object setting up its properties
     * that are given as parameters.
     *
     * @param name                  Name of the switch
     * @param maxPacketSize         Maximum packet size supported by the switch
     * @param timeToTravel          Time that a packet takes to leave its port and reach the destination
     * @param transmissionTime      Time taken to process the packet inside the switch
     * @param portSpeed             Transmission speed of the port
     * @param gbSize                Size of the guard bands used to separate non consecutive time slots
     */
    TSNSwitch(std::string name,
                     float maxPacketSize,
                     float timeToTravel,
                     float transmissionTime,
                     float portSpeed,
                     float gbSize,
                     float cycleDurationLowerBound,
                     float cycleDurationUpperBound) {
        this->name = name;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->transmissionTime = transmissionTime;
        this->portSpeed = portSpeed;
        this->gbSize = gbSize;
        this->ports.clear();
        this->connectsTo.clear();
        this->cycleDurationLowerBound = cycleDurationLowerBound;
        this->cycleDurationUpperBound = cycleDurationUpperBound;
    }

    /**
     * [Method]: TSNSwitch
     * [Usage]: Overloaded constructor method of this class.
     * Instantiates a new TSNSwitch object setting up its properties
     * that are given as parameters. There is no transmission time here,
     * as this method is used when considering that it will be calculated
     * by the packet size divided by the port speed
     *
     * @param name                  Name of the switch
     * @param maxPacketSize         Maximum packet size supported by the switch
     * @param timeToTravel          Time that a packet takes to leave its port and reach the destination
     * @param portSpeed             Transmission speed of the port
     * @param gbSize                Size of the guard bands used to separate non consecutive time slots
     */
    TSNSwitch(std::string name,
                     float maxPacketSize,
                     float timeToTravel,
                     float portSpeed,
                     float gbSize,
                     float cycleDurationLowerBound,
                     float cycleDurationUpperBound) {
        this->name = name;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->transmissionTime = 0;
        this->portSpeed = portSpeed;
        this->gbSize = gbSize;
        this->ports.clear();
        this->connectsTo.clear();
        this->cycleDurationLowerBound = cycleDurationLowerBound;
        this->cycleDurationUpperBound = cycleDurationUpperBound;
    }


    /**
     * [Method]: toZ3
     * [Usage]: After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
    void toZ3(context& ctx, solver solver) {
        this->cycleDurationLowerBoundZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(cycleDurationLowerBound).c_str()));
        this->cycleDurationUpperBoundZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(cycleDurationUpperBound).c_str()));

        // Creating the cycle duration and start for this switch
        this->cycleDuration = std::make_shared<expr>(ctx.real_const((std::string("cycleOf") + this->name + std::string("Duration")).c_str()));
        this->cycleStart = std::make_shared<expr>(ctx.real_const((std::string("cycleOf") + this->name + std::string("Start")).c_str()));


        // Creating the cycle setting up the bounds for the duration (Cycle duration constraint)
        solver.add(*this->cycleDuration >= *this->cycleDurationLowerBoundZ3);
        solver.add(*this->cycleDuration <= *this->cycleDurationUpperBoundZ3);

        // A cycle must start on a point in time, so it must be greater than 0
        solver.add(*this->cycleStart >= ctx.int_val(0)); // No negative cycle values constraint

        for (Port *port : this->ports) {
            port->toZ3(ctx);

            for(FlowFragment *frag : port->getFlowFragments()) {
                solver.add(*port->getCycle()->getFirstCycleStartZ3() <= *this->arrivalTime(ctx, 0, frag)); // Maximum cycle start constraint
            }

            solver.add(*port->getCycle()->getFirstCycleStartZ3() >= ctx.int_val(0)); // No negative cycle values constraint

            /* The cycle of every port must have the same duration
            solver.add(mkEq( // Equal cycle constraints
                this->cycleDuration,
                port.getCycle()->getCycleDurationZ3()
            ));
            /**/

            // The cycle of every port must have the same starting point
            /**/
            solver.add(*this->cycleStart == *port->getCycle()->getFirstCycleStartZ3()); // Equal cycle constraints
            /**/

        }

        solver.add(*this->cycleStart == ctx.int_val(0));

    }

    /**
     * [Method]: setupSchedulingRules
     * [Usage]: Iterates over the ports of the switch, calling the
     * method responsible for setting up the rules for each individual port
     * on the switch.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setupSchedulingRules(solver solver, context& ctx) {

        for(Port *port : this->ports) {
                port->setupSchedulingRules(solver, ctx);
        }

    }

    /**
     * [Method]: createPort
     * [Usage]: Adds a port to the switch. A cycle to that port and
     * the device object that it connects to (since TSN ports connect to
     * individual nodes in the approach of this schedule) must be given
     * as parameters.
     *
     * @param destination       Destination of the port as TSNSwitch or Device
     * @param cycle             Cycle used by the port
     */
    void createPort(cObject *destination, Cycle *cycle) {

        if(dynamic_cast<Device *>(destination)) {
            this->connectsTo.push_back(((Device *)destination)->getName());
            this->ports.push_back(
                    new Port(this->name + std::string("Port") + std::to_string(this->portNum),
                            this->portNum,
                            ((Device *)destination)->getName(),
                            this->maxPacketSize,
                            this->timeToTravel,
                            this->transmissionTime,
                            this->portSpeed,
                            this->gbSize,
                            cycle
                    )
            );
        } else if (dynamic_cast<Switch *>(destination)) {
            this->connectsTo.push_back(((Switch *)destination)->getName());

            Port *newPort = new Port(this->name + std::string("Port") + std::to_string(this->portNum),
                    this->portNum,
                    ((Switch *)destination)->getName(),
                    this->maxPacketSize,
                    this->timeToTravel,
                    this->transmissionTime,
                    this->portSpeed,
                    this->gbSize,
                    cycle
            );

            newPort->setPortNum(this->portNum);

            this->ports.push_back(newPort);
        }
        else
            ; // [TODO]: THROW ERROR





        this->portNum++;
    }

    /**
     * [Method]: createPort
     * [Usage]: Adds a port to the switch. A cycle to that port and
     * the device name that it connects to (since TSN ports connect to
     * individual nodes in the approach of this schedule) must be given
     * as parameters.
     *
     * @param destination       Name of the destination of the port
     * @param cycle             Cycle used by the port
     */
    void createPort(std::string destination, Cycle *cycle) {
        this->connectsTo.push_back(destination);

        this->ports.push_back(
                new Port(this->name + std::string("Port") + std::to_string(this->portNum),
                        this->portNum,
                        destination,
                        this->maxPacketSize,
                        this->timeToTravel,
                        this->transmissionTime,
                        this->portSpeed,
                        this->gbSize,
                        cycle
                )
        );

        this->portNum++;
    }


    /**
     * [Method]: addToFragmentList
     * [Usage]: Given a flow fragment, it finds the port that connects to
     * its destination and adds the it to the fragment list of that specific
     * port.
     *
     * @param flowFrag      Fragment of a flow to be added to a port
     */
    void addToFragmentList(FlowFragment *flowFrag);

    /**
     * [Method]: getPortOf
     * [Usage]: Given a name of a node, returns the port that
     * can reach this node.
     *
     * @param name      Name of the node that the switch is connects to
     * @return          Port of the switch that connects to a given node
     */
    Port *getPortOf(std::string name) {
        int index = std::find(connectsTo.begin(), connectsTo.end(), name) - connectsTo.begin();

        // System.out.println(std::string("On switch " + this->getName() + std::string(" looking for port to ")) + name);

        Port *port = this->ports.at(index);

        return port;
    }

    /**
     * [Method]: setUpCycleSize
     * [Usage]: Iterate over its ports. The ones using automated application
     * periods will calculate their cycle size.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setUpCycleSize(solver solver, context& ctx) {
        for(Port *port : this->ports) {
            port->setUpCycle(solver, ctx);
        }
    }


    /**
     * [Method]: arrivalTime
     * [Usage]: Retrieves the arrival time of a packet from a flow fragment
     * specified by the index given as a parameter. The arrival time is the
     * time when a packet reaches this switch's port.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex      Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> arrivalTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    /**
     * [Method]: arrivalTime
     * [Usage]: Retrieves the arrival time of a packet from a flow fragment
     * specified by the index given as a parameter. The arrival time is the
     * time when a packet reaches this switch's port.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex      Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     *
    std::shared_ptr<expr> arrivalTime(context& ctx, z3::expr index, FlowFragment flowFrag){
    int portIndex = this->connectsTo.indexOf(flowFrag->getNextHop());
    return (z3::expr) this->ports.get(portIndex).arrivalTime(ctx, index, flowFrag);
    }
    /**/

    /**
     * [Method]: departureTime
     * [Usage]: Retrieves the departure time of a packet from a flow fragment
     * specified by the index given as a parameter. The departure time is the
     * time when a packet leaves its previous node with this switch as a destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param index         Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> departureTime(context& ctx, z3::expr index, FlowFragment *flowFrag);
    /**/

    /**
     * [Method]: departureTime
     * [Usage]: Retrieves the departure time of a packet from a flow fragment
     * specified by the index given as a parameter. The departure time is the
     * time when a packet leaves its previous node with this switch as a destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex         Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> departureTime(context& ctx, int auxIndex, FlowFragment *flowFrag);
    /**
     * [Method]: scheduledTime
     * [Usage]: Retrieves the scheduled time of a packet from a flow fragment
     * specified by the index given as a parameter. The scheduled time is the
     * time when a packet leaves this switch for its next destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param index         Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the scheduled time of the desired packet
     *
    std::shared_ptr<expr> scheduledTime(context& ctx, z3::expr index, FlowFragment flowFrag){
    int portIndex = this->connectsTo.indexOf(flowFrag->getNextHop());
    return (z3::expr) this->ports.get(portIndex).scheduledTime(ctx, index, flowFrag);
    }
    /**/

    /**
     * [Method]: scheduledTime
     * [Usage]: Retrieves the scheduled time of a packet from a flow fragment
     * specified by the index given as a parameter. The scheduled time is the
     * time when a packet leaves this switch for its next destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex         Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the scheduled time of the desired packet
     */
    std::shared_ptr<expr> scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    void loadZ3(context& ctx, solver solver) {
        /*
        solver.add(
            mkEq(
                this->cycleDurationUpperBoundZ3,
                ctx.real_val(std::to_string(this->cycleDurationUpperBound))
            )
        );

        solver.add(
            mkEq(
                this->cycleDurationLowerBoundZ3,
                ctx.real_val(std::to_string(this->cycleDurationLowerBound))
            )
        );
        */

        if(!ports.empty()) {
            for(Port *port : this->ports) {
                //System.out.println(port.getIsModifiedOrCreated());
                port->loadZ3(ctx, solver);

            }
        }

    }


    /*
     *  GETTERS AND SETTERS
     */

    Cycle *getCycle(int index) {

        return this->ports.at(index)->getCycle();
    }

    void setCycle(Cycle *cycle, int index) {
        this->ports.at(index)->setCycle(cycle);
    }

    float getGbSize() {
        return gbSize;
    }

    void setGbSize(float gbSize) {
        this->gbSize = gbSize;
    }

    std::shared_ptr<expr> getGbSizeZ3() {
        return gbSizeZ3;
    }

    void setGbSizeZ3(z3::expr gbSizeZ3) {
        this->gbSizeZ3 = std::make_shared<expr>(gbSizeZ3);
    }

    std::vector<Port *> getPorts() {
        return ports;
    }

    void setPorts(std::vector<Port *> ports) {
        this->ports = ports;
    }

    void addPort(Port *port, std::string name) {
        this->ports.push_back(port);
        this->connectsTo.push_back(name);
    }

    std::shared_ptr<expr> getCycleDuration() {
        return cycleDuration;
    }

    void setCycleDuration(z3::expr cycleDuration) {
        this->cycleDuration = std::make_shared<expr>(cycleDuration);
    }

    std::shared_ptr<expr> getCycleStart() {
        return cycleStart;
    }

    void setCycleStart(z3::expr cycleStart) {
        this->cycleStart = std::make_shared<expr>(cycleStart);
    }

    bool getIsModifiedOrCreated() {
        return isModifiedOrCreated;
    }

    void setIsModifiedOrCreated(bool isModifiedOrCreated) {
        this->isModifiedOrCreated = isModifiedOrCreated;
    }

    std::vector<std::string> getConnectsTo(){
        return this->connectsTo;
    }
};

}

#endif

