#ifndef _ISTORAGEDEVICE_H_
#define _ISTORAGEDEVICE_H_

#include "inet/icancloud/Architecture/NodeComponents/Hardware/HWEnergyInterface.h"
#include "inet/icancloud/Architecture/NodeComponents/Hardware/Storage/StorageController/StorageController.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"
#include "inet/icancloud/Base/include/Constants.h"

namespace inet {

namespace icancloud {


class StorageController;

/**
 * @class SimpleDisk SimpleDisk.h "SimpleDisk.h"
 *   
 * Class that simulates a disk with a user defined bandwidth (for reads and writes)
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 *
 * @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
 * @date 2012-23-11
 *
 */
class IStorageDevice: public HWEnergyInterface{


	protected:
				  	
	  	/** Read bandwidth */
	  	unsigned int readBandwidth = 0;
	  	
	  	/** Write bandwidth */
	  	unsigned int writeBandwidth = 0;
	
		/** Gate ID. Input gate. */
		cGate* inGate = nullptr;
		
		/** Gate ID. Output gate. */
		cGate* outGate = nullptr;
	    	    
	    /** Node state */
	    string nodeState;

	    StorageController* storageMod = nullptr;

		/** Request time. */
		simtime_t requestTime;
	
		/** Pending message */
	  //  cMessage *pendingMessage;

	    

      /**
       *  Module initialization.
       */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
	    
	    void e_changeState (const string &energyState) override;

public:
	   /**
	 	* Module ending. 		
	 	*/ 
	    virtual void finish() override = 0;
	
	
	   /**
		* Get the outGate to the module that sent <b>msg</b>
		* @param msg Arrived message.
		* @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
		*/ 
	    virtual cGate* getOutGate (cMessage *msg) override = 0;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
	    virtual void processSelfMessage (cMessage *msg) override = 0;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
	    virtual void processRequestMessage (Packet *) override = 0;

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
	    virtual void processResponseMessage (Packet *) override = 0;
		
	   /**
 		* Method that calculates the spent time to process the current request.
 		* @param sm Message that contains the current I/O request.
 		* @return Spent time to process the current request. 
 		*/
	    virtual simtime_t service (Packet *) = 0;
		
				/*
		 * Change the energy state of the memory given by node state
		 */
	    virtual void changeDeviceState (const string & state, unsigned componentIndex) override = 0;
	    virtual void changeDeviceState (const string & state) override {changeDeviceState (state, 0);}

		/*
		 * Change the energy state of the disk
		 */
	    virtual void changeState (const string & energyState, unsigned componentIndex) override = 0;
        virtual void changeState (const string & energyState) override {changeState (energyState, 0);}

};

} // namespace icancloud
} // namespace inet

#endif

