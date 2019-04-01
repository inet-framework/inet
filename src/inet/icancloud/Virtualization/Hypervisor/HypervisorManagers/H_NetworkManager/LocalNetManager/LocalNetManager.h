//
// This class defines a Manager responsible for translate virtual ports and virtual ip address to real physical ports and ip address.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


#ifndef LOCALNET_MANAGER_H_
#define LOCALNET_MANAGER_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/LocalNetManager/PAT/PortAddressTranslation.h"
#include "inet/icancloud/Management/NetworkManager/NetworkManager.h"

#define ROOT_PORT 0                     /** Root port */
#define REGISTERED_INITIAL_PORT 1024    /** Initial Registered port number */
#define DYNAMIC_INITIAL_PORT 49152      /** Initial dynamic port number */
#define LAST_PORT 65535                 /** Last dynamic port number */

namespace inet {

namespace icancloud {


class LocalNetManager : public icancloud_Base {

protected:

	PortAddressTranslation* pat;
	L3Address ip_LocalNode;

	NetworkManager* netManagerPtr;
public:

	virtual ~LocalNetManager();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

	void finish() override;
   /**
	* Get the out Gate to the module that sent <b>msg</b>.
	* @param msg Arrived message.
	* @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
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


	void initializePAT (const L3Address &nodeIP);

	void createVM(Packet* sm);

	/*
	 *  Create an entry into the ports connections
	 */
	void manage_listen(Packet* sm);

	/*
	 * This method change a connection creation message from virtual layer to physical layer translating
	 * the ip addresses virtual to physical. The message operation is to connect this node with a storage node
	 */
	int manage_create_storage_Connection(Packet* sm);

	/*
	 * This method change a connection creation message from virtual layer to physical layer translating
     * the ip addresses virtual to physical
     */
	int manage_createConnection(Packet* sm);

	/*
	 *  When the connection is stablished, the client receives a port to the connection
	 *  that has to be allocated into the structure
	 */
	void connectionStablished(Packet* sm);

	/*
	 * Erase all the data from the structures of a virtual machine..
	 */
	vector<Packet*> manage_close_connections(int uId, int pId);

	/*
	 * Translate from real to virtual port
	 */
	void manage_receiveMessage(Packet* sm);

	/*
	 * Translate from virtual to real port
	 */
	void manage_sendMessage(Packet* sm);

	/*
	 * Erase the data from the structures associated to a connection
	 */
	void manage_close_single_connection(Packet* sm);


private:

	/*
	 * Get all the connections id from the pat
	 */
		vector<int> getConnectionsIDs(int uId, int pId);

};

} // namespace icancloud
} // namespace inet

#endif /* USER_VIRTUALPORT_CELL_H_ */
