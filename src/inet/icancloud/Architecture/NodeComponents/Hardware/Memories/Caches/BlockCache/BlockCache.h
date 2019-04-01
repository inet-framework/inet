#ifndef __BLOCK_CACHE_H_
#define __BLOCK_CACHE_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {



/**
 * @class BlockCache BlockCache.h "BlockCache.h"
 *   
 * nullptr Cache.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 */
class BlockCache :public icancloud_Base{


	protected:		
		
		/** Number of inputs */
		unsigned int numInputs;
		
		/** Cache hit ratio */
	    double hitRatio;    		
	  
	    /** Gate ID. Output gate to Inputs gates. */
	    cGate* toInputGate;
	    
	    /** Gate ID. Input gate from Input gates. */
	    cGate* fromInputGate;
	    
	    /** Gate ID. Output gate to Output. */
	    cGate* toOutputGate;
	    
	    /** Gate ID. Input gate from Output. */
	    cGate* fromOutputGate;	    	        	        
	        	
	        	
	   /**
	    * Destructor.
	    */    		
	    ~BlockCache();			
	  	        			  	    	    
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
		* Get the outGate ID to the module that sent <b>msg</b>
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
