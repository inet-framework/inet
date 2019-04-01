#ifndef _E_NETWORK_SERVICE_H_
#define _E_NETWORK_SERVICE_H_

#include <omnetpp.h>
#include "inet/icancloud/Architecture/NodeComponents/Hardware/HWEnergyInterface.h"
#include "inet/icancloud/Base/Messages/icancloud_App_NET_Message.h"

#define     NETWORK_ON                  std::string("network_on")                    // Network OFF
#define     NETWORK_OFF                 std::string("network_off")                   // Network ON

namespace inet {

namespace icancloud {

/**
 * @class NetworkService NetworkService.h "NetworkService.h"
 *   
 * Network Service Module
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 *
 * @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-10-23
 */


class TCP_ClientSideService;
class TCP_ServerSideService;

class NetworkService : public HWEnergyInterface{


	protected:	
		
		/** Local IP address */
		string localIP;
										
		/** TCP Client-side Services */
		TCP_ClientSideService *clientTCP_Services;
		
		/** TCP Server-side Services */
		TCP_ServerSideService *serverTCP_Services;
		
		/** Input gate from Service Redirector */
	    cGate* fromNetManagerGate;

	    /** Input gate from Network (TCP) */
	    cGate* fromNetTCPGate;

	    /** Output gate to Service Redirector */
	    cGate* toNetManagerGate;

	    /** Output gate to Network (TCP) */
	    cGate* toNetTCPGate;	
	    
	    /** Node state */
	    string nodeState;

        vector <Packet*> sm_vector;
        int lastOp;

	    /**
	    * Destructor.
	    */    		
	    ~NetworkService();		
	  	        			  	    	    
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
		* This method classifies an incoming message and invokes the corresponding method
		* to process it.
		*
		* For self-messages, it invokes processSelfMessage(sm);
		* For request-messages, it invokes processRequestMessage(sm);
		* For response-messages, it invokes processResponseMessage(sm);
		*/
		void handleMessage (cMessage *msg) override;
	
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
		
		/**
		 * Receiving an established connection message.
		 */
		void receivedEstablishedConnection (cMessage *msg);


		/*
		 * Change the energy state of the memory given by node state
		 */
	     virtual void changeDeviceState (const string &state,unsigned int componentIndex) override;
	     virtual void changeDeviceState (const string &state) override {changeDeviceState(state,0);}

		/*
		 *  Change the energy state of the memory
		 */
	     virtual void changeState (const string &energyState,unsigned int componentIndex) override;
	     virtual void changeState (const string &energyState) override {changeState (energyState, 0);}
};

} // namespace icancloud
} // namespace inet

#endif
