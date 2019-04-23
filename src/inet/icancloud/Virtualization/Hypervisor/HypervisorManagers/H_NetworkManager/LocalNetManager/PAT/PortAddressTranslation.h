//
// This class defines a port address translation
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//
#ifndef PORT_MANAGER_H_
#define PORT_MANAGER_H_

#include "User_VirtualPort_Cell.h"


/** Port operations */
#define NOT_STABLISHED false
#define STABLISHED true

#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class PortAddressTranslation {

protected:

	bool freePorts [LAST_PORT];
	vector<int> portHoles;
	int portPtr;

	vector<User_VirtualPort_Cell*> user_vm_ports;

public:

	PortAddressTranslation();
	virtual ~PortAddressTranslation();

	void pat_initialize(const L3Address &nodeIP);

	void portForwarding(Packet *msg);

	void pat_createVM (int userID, int vmID, string vmIP);

private:

	// Auxiliar operations
		User_VirtualPort_Cell* searchUser (int uId);
		User_VirtualPort_Cell* newUser (int uId);
		void deleteUser (int pId);
	// Operations with ports
		int getNewPort();
		void freePort(int port);
public:
	// Operations from messages..

		/*
		 *  Create an entry into the ports connections
		 *  Returns the real port
		 */
		int pat_createListen(int uId, int pId, int virtualPort);

		/*
		 *  When the connection is stablished, the client receives a port to the connection
		 *  that has to be allocated into the structure
		 */
		int pat_connectionStablished(Ptr<icancloud_Message> &sm);

		/*
		 *  Complete the connection for a listen arrival
		 */
		void pat_arrivesIncomingConnection(Ptr<icancloud_Message> &sm);

		/*
		 * Translate from real to virtual port
		 */
		void pat_receiveMessage(Ptr<icancloud_Message> &sm);

		/*
		 * Translate from virtual to real port
		 */
		void pat_sendMessage(Ptr<icancloud_Message> &sm);

		/*
		 * Erase the data from the structures
		 * Returs the virtual ip
		 */
		int pat_closeConnection(Ptr<icancloud_Message> &sm);

		/*
		 * Erase all data from the structures
		 */
		vector<int> pat_closeVM(int uId, int pId);

};

} // namespace icancloud
} // namespace inet

#endif /* USER_VIRTUALPORT_CELL_H_ */
