//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef H_STORAGEMAN_BASE_H_
#define H_STORAGEMAN_BASE_H_

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/HW_Cells/AbstractStorageCell.h"

namespace inet {

namespace icancloud {


class H_StorageManager_Base  : virtual public icancloud_Base{

protected:

		int numStorageServers;										// Number of Block Servers
		int nodeGate;

		cGateManager* fromVMStorageServers;
		cGateManager* toVMStorageServers;
		cGate** fromNodeStorageServers;
		cGate** toNodeStorageServers;

		cGate* toNET_Manager;
		cGate* fromNET_Manager;

		cGateManager* from_storageCell;
		cGateManager* to_storageCell;

	    /** Array to show the ID of VM executing in the core 'i'*/
	    struct storageControl_t{
	        int gate;
	        int uId;
	        int pId;
	        int storageSizeKB;
	        cModule* remoteStorage;
	    };
	    typedef storageControl_t storageControl;

	    vector<storageControl*> vmIDs;

	    cModule* originalStorage;


	    // Overhead

	    struct overhead_t{
           Packet* msg;
           simtime_t  timeStamp;
        };

        typedef overhead_t overhead;

	    vector<overhead*> overheadStructure;

	    double io_overhead;


protected:

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
        * Get the out Gate to the module that sent <b>msg</b>.
        * @param msg Arrived message.
        * @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
        */
		cGate* getOutGate (cMessage *msg) override;

	   /**
		* Process a response message.
		* @param sm Request message.
		*/
		void processResponseMessage (Packet *) override;

		/*
		 * Process an IO response
		 */
	    void processIOCallResponse (Packet *sm_io);

public:

	virtual ~H_StorageManager_Base();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

	virtual void finish() override;

    void setVM (cGate* oGate, cGate* iGate, int uId, int pId, int requestedStorageGB);

    void freeVM (int uId, int pId);

protected:

    /*
     * Virtual method to be implemented at scheduler.
     */
    virtual void schedulingStorage(Packet *msg) = 0;

    cModule* createStorageCell (int uId, int pId, int64_t storageSizeGB);

    void setRemoteStorage(Packet* sm_io);

    int setControlStructure(int uId, int pId, int gate,int64_t storageSizeKB);

    int getCellGate(int uId, int pId);

	Packet* migrationToDiskContents(Packet* sm);

	void processMigrationReconnections(Packet *sm_migration);

	void processMigrationMessage(Packet* msg);

	long int updateOperation(int uId, int pId, int64_t sizeKB, bool decrease);

};

} // namespace icancloud
} // namespace inet

#endif /* H_BSMANAGER_BASE_H_ */
