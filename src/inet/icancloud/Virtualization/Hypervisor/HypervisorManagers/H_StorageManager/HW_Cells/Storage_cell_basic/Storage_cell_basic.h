//
// class Storage_cell_basic.h defines a generic storage cell
//
// A storage cell is responsible for the remote storage system.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


#ifndef STORAGE_CELL_BASIC_H_
#define STORAGE_CELL_BASIC_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/HW_Cells/AbstractStorageCell.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/FsType/NFS_Storage_Cell.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/FsType/PFS_Storage_Cell.h"

namespace inet {

namespace icancloud {


class Storage_cell_basic : public AbstractStorageCell {

protected:

	Packet* pendingMessage = nullptr;

public:

	virtual ~Storage_cell_basic();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    
   /**
	* Process a self message.
	* @param msg Self message.
	*/
	virtual void processSelfMessage (cMessage *msg) override;;

   /**
	* Process a request message.
	* @param sm Request message.
	*/
	virtual void processRequestMessage (Packet *) override;

   /**
	* Process a response message.
	* @param sm Request message.
	*/
	virtual void processResponseMessage (Packet *) override;


	bool setRemoteData (Packet*) override;

private:
	struct replica_T{
	    int commId;
	    int uId;
	    int pId;
	    int machinesleft;
	    Packet* sms;
	};

	typedef replica_T replica_t;

	vector<replica_t*> replicas;

};

} // namespace icancloud
} // namespace inet

#endif /* STORAGE_CELL_BASIC_H_ */
