//
// class that defines the behavior of a network hypervisor manager.
//
// It is composed by two main parts:
//  - Local net manager: Manager responsible for translate virtual ports and virtual ip address to real physical ports and ip address
//  - network manager: Manager that manages the order of arrival messages from VMs to access to physical resources
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


#ifndef H_NETMANAGER_BASE_H_
#define H_NETMANAGER_BASE_H_

#include <omnetpp.h>

#include "inet/icancloud/Base/include/icancloud_debug.h"
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/include/Constants.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/LocalNetManager/LocalNetManager.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

namespace icancloud {


class H_NETManager_Base  : public icancloud_Base{

protected:

		int nodeGate;

		cGateManager* fromVMNet;
		cGateManager* toVMNet;
		cGate* fromNodeNet;
		cGate* toNodeNet;

		cGate* fromHStorageManager;
		cGate* toHStorageManager;

		string ipNode;

		struct vmControl_t{
		    int gate;
		    int uId;
		    int pId;
			string virtualIP;
		};

		typedef vmControl_t vmControl;

		vector <vmControl*> vms;

		int storageApp_ModuleIndex;

	    LocalNetManager* localNetManager;

	    struct enqueuedMessage{
	    	int timesEnqueued;
	    	Packet* sms;
	    	double time_enqueued;
	    };

	    vector<enqueuedMessage*> pendingConnections;
	    int connectionTimeout;


        // Overhead
        struct overhead_t{
           Packet* msg;
           simtime_t  timeStamp;
        };

        typedef overhead_t overhead;

        vector<overhead*> overheadStructure;

        double net_overhead;

public:
        /*
         * Scheduling process to be defined at scheduler.
         */
        virtual void schedulingNET (Packet *sm) = 0;

        /*
         * This method sets the input and output gates from vm to the network manager
         */
        void setVM(cGate* oGate, cGate* iGate, int uId, int pId, string virtualIP, int requiredNetIf);

        /*
         * This method delete the gates connected to the VM with id given as parameter
         */
        void freeVM(int uId, int pId);

        /*
         * This method returns the quantity of virtual machines allocated at node
         */
        int getNumberOfVMs(){return vms.size();};


protected:

	    /*
	     * Destructor
	     */
	    virtual ~H_NETManager_Base();

	    /*
	     * Module initialization
	     */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        

	    /*
	     * Module finalization
	     */
	    void finish() override;

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

		/*
		 *
		 */
		cGate* getOutGate (cMessage *msg) override;

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;

	    /*
	     * This method enqueue the message until the connection will be created
	     */
	    void enqueuePendingMessage(Packet*);

	    /*
	     * This method check if there is any message enqueued
	     */
	    void checkPendingMessages();


protected:

        /*
         * This method returns the gate by a given uId and pId from the user and virtual machine
         */
        int getGateByID (int uId, int pId);

        /*
         * This method returns the gate by a given virtualIP from the user and virtual machine
         */
        int getGateByVIP (string virtualIP, int uId);

};

} // namespace icancloud
} // namespace inet

#endif /* H_BSMANAGER_BASE_H_ */
