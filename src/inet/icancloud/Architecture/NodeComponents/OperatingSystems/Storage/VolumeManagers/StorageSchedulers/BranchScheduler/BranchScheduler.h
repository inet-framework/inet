#ifndef __BRANCH_SCHEDULER_H_
#define __BRANCH_SCHEDULER_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/Messages/SMS/SMS_Branch.h"
#include "inet/icancloud/Base/Messages/icancloud_BlockList_Message.h"

namespace inet {

namespace icancloud {



/**
 * @class BranchScheduler BranchScheduler.h "BranchScheduler.h"
 * 
 * This module schedules I/O requests.
 * Derived classes must specify the schedulling policies.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-12
 *
 * @author Gabriel González Castañé
 * @date 2015-01-26
 */
class BranchScheduler: public icancloud_Base{
		
	protected:  	  	
				 
	    /** Output gate to Input gate. */
	    cGate* toInputGate;
	    
	    /** Input gate from Input gate. */
	    cGate* fromInputGate;
	    
	    /** Output gate. */
	    cGate* toOutputGate;
	    
	    /** Input gate. */
	    cGate* fromOutputGate;	    	        
	    
	    /** SplittingMessageSystem Object*/
	    SMS_Branch *SMS_branch;	
		
	   /**
		* Destructor 
		*/
		~BranchScheduler();
	
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
		
	   /**
		* Process all pending branches	
		*/
		void processBranches ();		
		
	   /**
		* Send a request message to its destination!
		* @param sm Request message.
		* @param gate Gate used to send the message.
		*/		
		void sendRequestMessage (Packet *, cGate* gate) override;
};

} // namespace icancloud
} // namespace inet

#endif
