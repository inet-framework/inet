/**
 * @class Machine Machine.h Machine.cc
 *
 * This class models the base of an execution unit. It can be a Node, VM, or any hardware
 * set of components that can execute applications
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-12-12
 */
#ifndef _MACHINE_H_
#define _MACHINE_H_

#include "inet/icancloud/Management/MachinesStructure/ElementType.h"
#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SyscallManager/AbstractSyscallManager.h"

namespace inet {

namespace icancloud {


class Machine : virtual public icancloud_Base{

protected:

    elementType* type;                                 // Contains all the features of a Node
    string ip;                                         // IP
    AbstractSyscallManager* os;                        // Pointer to operative system

    /** Pointer to the manager of the system*/
    icancloud_Base* managerPtr;                        // Pointer to manager of the system

protected:

    /*
     * Destructor
     */
    virtual ~Machine();

    /*
     * Method that initializes the module
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * Method that finalizes the module
     */
    virtual void finish() override;

    /**
    * Get the outGate to the module that sent <b>msg</b>
    * @param msg Arrived message.
    * @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
    */
    virtual cGate* getOutGate (cMessage *msg) override { return nullptr;};

    /**
    * Process a self message.
    * @param msg Self message.
    */
    virtual void processSelfMessage (cMessage *msg) override {};

    /**
    * Process a request message.
    * @param sm Request message.
    */
    virtual void processRequestMessage (Packet *) override {};

    /**
    * Process a response message.
    * @param sm Request message.
    */
    virtual void processResponseMessage (Packet *) override{};


public:

    // --------------------------------------------------------------------------/
    // ----------------- Operations with attributes of the node -----------------/
    // --------------------------------------------------------------------------/

    //------------------ To operate with the physical attributes of the node -------------
        // Getters

            // Returns the free memory of the node
            virtual double getFreeMemory (){return os->getFreeMemory();};

            // Get the total memory size of the node
            int getMemoryCapacity(){return  type->getMemorySize();};

            // Returns the free storage of the node
            virtual double getFreeStorage (){return os->getFreeStorage();};

            // Returns the total amount of storage capacity of the node
            double getStorageCapacity (){return type->getStorageSize();};

            // Get the total number of CPUs
            int getNumCores(){return type->getNumCores();};

            // Get the number of network interfaces
            int getNumNetworkIF (){return type->getNumNetIF();};

            // Get the ip of the node
            string getIP (){return ip;};
            void setIP(string i) {ip = i;};

            // Getter for type the element type name
            string getTypeName (){return type->getType();};
            elementType* getElementType(){return type;};

            // Geter for the quantity of different storage devices that are at machine
            int getNumOfStorageDevices(){return 1;};

            // Returns the unique identifier given by omnet to this module
            int getPid(){return this->getId();};

    // -----------------------------------------------------------------------/
    // ----------------- Operations with states ------------------------------/
    // -----------------------------------------------------------------------/

    /*
     * This method checks if the state of the machine is not off
     */
    bool isON(){return (!equalStates(getState().c_str(),MACHINE_STATE_OFF));};   // This method returns the state attribute

    /*
     * This method checks if the state is off
     */
    bool isOFF(){return (equalStates(getState().c_str(),MACHINE_STATE_OFF));};   // This method returns the state attribute

    /*
     * This method will return the state of the machine between MACHINE_STATE_OFF, MACHINE_STATE_IDLE and MACHINE_STATE_RUNNING
     */
    string getState(){return os->getState();};

    /*
     * This method changes the state of the machine to MACHINE_STATE_ON. To be implemented at super class
     */
    virtual void turnOn(){os->changeState(MACHINE_STATE_IDLE);};

    /*
     * This method changes the state of the machine to MACHINE_STATE_OFF. To be implemented at super class
     */
    virtual void turnOff(){os->changeState(MACHINE_STATE_OFF);};

    /*
     * Change the state of the machine to the given as parameter
     */
    virtual void changeState (string newState){os->changeState(newState);};


    // -----------------------------------------------------------------------/
    // ----------------- Operations with applications ------------------------/
    // -----------------------------------------------------------------------/

    /*
     * Setter for manager
     */
    virtual void setManager(icancloud_Base* manager){managerPtr = manager;};

    /*
     * This method will allocate a job into the node
     */
    void allocateProcess(icancloud_Base* job, simtime_t timeToStart, int uId){os->allocateJob(job,timeToStart,uId);};

    /*
     * Check if an application is executing at system
     */
    bool isAppRunning (int pId){return os->isAppRunning(pId);};

    /*
     * This method will return the number of processes (Only user processes) executing at node
     */
    virtual int getNumProcessesRunning(){return os->getNumProcessesRunning();};

    /*
     * This method will return the process at the position given allocated at os structure.
     */
    icancloud_Base* getProcessRunning(int index){return os->getProcessRunning(index);};

    /*
     * Method to compare two different states. It will return true if both states are the same one.
     */
    bool equalStates(const string & state1, const string & state2){return (strcmp(state1.c_str(), state2.c_str()) == 0);};

    /*
     * Unlink and destroy an application from the system
     */
    void removeProcess (int pId);

    /*
     * Unlink and destroy all applications from the system
     */
    void removeAllProcesses (){os->removeAllProcesses();};


};


} // namespace icancloud
} // namespace inet

#endif /* _ABSTRACTNODE_H_ */
