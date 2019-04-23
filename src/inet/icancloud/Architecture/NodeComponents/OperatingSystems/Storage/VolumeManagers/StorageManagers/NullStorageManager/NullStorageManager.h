#ifndef __nullptrSTORAGE_MANAGER_H_
#define __nullptrSTORAGE_MANAGER_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"
#include "inet/icancloud/Base/Messages/SMS/SMS_RAID_0.h"

namespace inet {

namespace icancloud {



/**
 * @class NullStorageManager NullStorageManager.h "NullStorageManager.h"
 *
 * This module calculates the location of the Storages involved in an I/O operation,
 * and send the corresponding requests to the corresponding Storage Server.
 *
 * If parameter <b>numStorageSystem</b> is greater than 1, this volume manager
 * will act like a RAID controller.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-12
 */
class NullStorageManager :public icancloud_Base{

	protected:

	    /** Number of Storage servers */
	    unsigned int numStorageSystem;


	    /** Output gate to Scheduler. */
	    cGate* toSchedulerGate;

	    /** Input gate from Scheduler. */
	    cGate* fromSchedulerGate;

	    /** Output gates to Storage Server. */
	    cGate** toStorageSystemGates;

	    /** Input gates from Storage Server. */
	    cGate** fromStorageSystemGates;

		

	   /**
		* Destructor
		*/
		~NullStorageManager();

	   /**
	 	*  Module initialization.
	 	*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        


	   /**
	 	* Module ending.
	 	*/
	    void finish() override;


	private:

	   /**
		* Get the outGate to the module that sent <b>msg</b>
		* @param msg Arrived message.
		* @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
		*/
		cGate* getOutGate (cMessage *msg) override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		void processSelfMessage (cMessage *msg) override;;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		void processRequestMessage (Packet *) override;

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;

};

} // namespace icancloud
} // namespace inet

#endif
