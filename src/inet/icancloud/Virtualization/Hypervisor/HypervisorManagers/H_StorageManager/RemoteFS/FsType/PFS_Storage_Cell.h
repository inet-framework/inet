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

#ifndef PFS_CELL_H_
#define PFS_CELL_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/Abstract_Remote_FS.h"
#include "inet/icancloud/Base/Messages/SMS/SMS_PFS.h"
#include "inet/icancloud/Base/Messages/SMS/SMS_NFS.h"

namespace inet {

namespace icancloud {


class PFS_Storage_Cell : public Abstract_Remote_FS{

	protected:

	    SMS_PFS *SMS_pfs;			/** SplittingMessageSystem Object*/
//	    SMS_NFS *SMS_nfs;			/** SplittingMessageSystem Object*/

		unsigned int strideSize;	/** Stride size (in bytes) */

	public:

	    PFS_Storage_Cell();
	    PFS_Storage_Cell(int n_strideSizeKB);
	    ~PFS_Storage_Cell();

	    virtual int getServerToSend(Packet*) override;
	    virtual Packet* hasMoreMessages() override;
	    virtual void initializeCell() override;
	    virtual bool splitRequest (cMessage *msg) override;
	    virtual Packet* popSubRequest () override;
	    virtual Packet* popNextSubRequest (cMessage* parentRequest) override;
	    virtual void arrivesSubRequest (cMessage* subRequest, cMessage* parentRequest) override;
	    virtual cMessage* arrivesAllSubRequest (cMessage* request) override;
	    virtual bool removeRequest (cMessage* request) override;

	private:
	    void initialize_servers_counters();
};

} // namespace icancloud
} // namespace inet

#endif /* NFS_CELL_H_ */
