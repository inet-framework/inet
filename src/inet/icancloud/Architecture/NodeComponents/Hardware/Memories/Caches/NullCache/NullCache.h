#ifndef __NULL_CACHE_H_
#define __NULL_CACHE_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {



/**
 * @class NullCache NullCache.h "NullCache.h"
 *   
 * nullptr Cache.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 */
class NullCache :public icancloud_Base{


	protected:		
		
		/** Number of inputs */
	    int numInputs;    		
	  
	    /** Gate IDs Vector. Output gates to Inputs gates. */
	    cGate** toInputGates;
	    
	    /** Gate IDs Vector. Input gates from Input gates. */
	    cGate** fromInputGates;
	    
	    /** Gate ID. Output gate to Output. */
	    cGate* toOutputGate;
	    
	    /** Gate ID. Input gate from Output. */
	    cGate* fromOutputGate;	    	        	        
	        	
	        	
	   /**
	    * Destructor.
	    */    		
	    ~NullCache();			
	  	        			  	    	    
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
