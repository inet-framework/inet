/**
 *
 * @class NetworkManager NetworkManager.cc NetworkManager.h
 *
 * The network manager is responsible for allocating all the virtualized ips. It know by a given node, which VMs (by ips) have allocated
 * and by a given Virtual IP where is it allocated.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-12-11
 */

#ifndef NETMANAGER_H_
#define NETMANAGER_H_

#include "inet/icancloud/Management/NetworkManager/VirtualIPCell.h"
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Management/NetworkManager/PortTable.h"

namespace inet {

namespace icancloud {


class NetworkManager : public icancloud_Base {

protected:

	// To manage the ips that will be given to the new VMs
	struct ipsUserSet{
		int userID;
		vector <string> holesIP;
		vector <VirtualIPCell*> virtualIPCell;
		string lastIP;
	};

	vector <ipsUserSet*> ipsCloud;
	string ipBasis;

	// The information of each port in all the nodes
	vector<PortTable*> ports_per_node;

	/*
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

public:

	virtual ~NetworkManager();

	void createNewUser (int newUserID);

	void deleteUser (int userID);

	void setIPBasis(string ip);

	string getIPBasis();

	string allocateVirtualIP (string ipNode, int userID, int vmID);

	void deleteVirtualIP_by_VIP(string virtualIP, int userID);

	void deleteVirtualIP_by_VMID(int vmID, int userID);

	string searchNodeIP (string virtualIP, int userID);

	string changeNodeIP (string virtualIP, int userID, string nodeIP);

	bool checkVMIP (string virtualIP, string nodeIP, int userID);

	int getUserIPSize();

	string searchVMIP (int userID, int vmID);

	// Operations with port table

	void registerNode (string nodeIP);

	void registerPort(int user, string ipNode, int vmID, int realPort, int virtualPort, int operation);

	int getRealPort(string destinationIP, int virtualPort, int destinationUser, int vmID);

	void freeAllPortsOfVM (string ipNode, int vmID, int user);

	void freePortOfVM (string ipNode, int vmID, int user, int virtualPort);

	int getVMid (string virtualIP, int uId);

private:

	string newVIP(ipsUserSet* user);

	bool isMajorIP(string nodeIP_A, string nodeIP_B);

	PortTable* getNodeFromPortsTable(string nodeIP);
};

} // namespace icancloud
} // namespace inet

#endif /* NETMANAGER_H_ */
