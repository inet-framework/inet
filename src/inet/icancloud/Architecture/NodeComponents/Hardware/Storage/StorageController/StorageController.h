//
// Module that represents a storage controller
//
// This module is responsible for controlling all storage devices of a machine
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
// @date 2012-23-11

// All the storage devices inherit from this class to cast the states from the Node!

#ifndef _STORAGE_SYSTEM_H_
#define _STORAGE_SYSTEM_H_

#include <omnetpp.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <sstream>
#include <vector>
#include <string>
#include "inet/icancloud/Architecture/NodeComponents/Hardware/Storage/Devices/IStorageDevice.h"

namespace inet {

namespace icancloud {


using std::string;
using std::pair;
using std::vector;

class IStorageDevice;

class StorageController  : public HWEnergyInterface{

  protected:

    // The number of cpu cores
    int numDevices;

    /** The other devices which complete the node storage system */
    vector<IStorageDevice*> complementaryDevices;

    /*
     * Destructor
     */
    virtual ~StorageController();

    /**
     *  Module initialization.
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /**
     * Module ending.
     */
     void finish() override;

     /**
      * Get the outGate ID to the module that sent <b>msg</b>
      * @param msg Arrived message.
      * @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
      */
      cGate* getOutGate (cMessage *msg) override;

     /**
      * Process a self message.
      * @param msg Self message.
      */
      void processSelfMessage (cMessage *msg) override {};

     /**
      * Process a request message.
      * @param sm Request message.
      */
      void processRequestMessage (Packet *) override {};

     /**
      * Process a response message.
      * @param sm Request message.
      */
      void processResponseMessage (Packet *) override {};

  public:

     /*
      * Constructor
      */
     StorageController();
     /*
      * Register cores
      */
     void registerDevice (IStorageDevice* device, int index);

	/*************************************
	 * Operations from HWManagerInterface
	 *************************************
	 */

    /**
     * Change the actual state to the new given state in the instant actualSimtime
     */
    void e_changeState (const string & state, unsigned componentIndex) override;

    /*
     * Return the actual state of the component. The parameter gives if there are several components
     */
    string e_getActualState (unsigned componentIndex) override;

    /*
     * Return the simtime associated to a given as parameter state
     */
    simtime_t e_getStateTime (const string & state, unsigned componentIndex) override;

    /*
     * This method returns the name of the state at position "statePosition" given as parameter
     */
    string e_getStateName (int statePosition, unsigned componentIndex) override;

    /*
     * This method returns the states vector as string.
     */
    string e_statesToString () {return "";};

    /*
     * Change the energy state of the memory given by node state
     */
    void changeDeviceState (const string & state, unsigned componentIndex) override;
    void changeDeviceState (const string &state) override {changeDeviceState (state, 0);}


    /*
     * Change the energy state of the disk
     */
    void changeState (const string & energyState, unsigned componentIndex) override;

};

} // namespace icancloud
} // namespace inet

#endif /* CPUMODULEINTERFACE_H_ */
