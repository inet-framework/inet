#ifndef __LIBTABLE_H
#define __LIBTABLE_H



#include <omnetpp.h>
#include <RoutingTableAccess.h> 
#include <ip_address_prefix.h>
#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include "StringTokenizer.h"
#include "ConstType.h"
#include <vector>

/*

Table examples:

LIB TABLE:



In-lbl      In-intf Out-lbl   Out-intf Optcode

L1			ppp0	L2			ppp3       1

L3			ppp1	L2			ppp4       1



PRT TABLE:

Fec		Pointer

F1		3

F3		5



Prefix TABLE:

Prefix		FEC

124.3.0.0	F1

135.6.1.0	F2



*/

struct prt_type{

	int pos;

	IPAddressPrefix fecValue; //Support prefix only, e.g 128.2.0.0

};

using namespace std;

struct lib_type{

	int inLabel;

	int outLabel;

	string inInterface;

	string outInterface;
	int optcode;

};

class LIBTable: public cSimpleModule
{

private:

	vector<lib_type> lib;
	vector<prt_type> prt;
	
	/**
	 * Load the label information table from files
	 * @param filename The lib table file input
	 * @return The successful or unsuccessful code
	 */
	int readLibTableFromFile(const char* filename);

	/**
	 * Load the Partial Routing Information from files
	 * @param filename The prt table file input
	 * @return The successfule or unsuccesful code
	 */
	int readPrtTableFromFile(const char* filename);


public:

	Module_Class_Members(LIBTable, cSimpleModule, 0);
	/**
	 * Initilize all the parameters for Lable Information Base processing 
	 * @param void
	 * @return void
	 */
	void initialize();
	
	/**
	 * Handle message from other module
	 * @param msg Message received
	 * @return void
	 */
    void handleMessage(cMessage *msg);

	/**
	 * Print out the contents of Label Information Base and Partial Routing Table
	 * @param void
	 * @return void
	 */
	void printTables();

	/**
	 * Install a new label on this Label Switching Router when receiving a label mapping from peers
	 * @param outLabel The label returned from peers
	          inInterface The name of the incoming interface of the label mapping
			  outInterface The name of the outgoing interface that new lable mapping will be forwarded
              fec The Forward Equivalent Class of the label
			  optcode The optcode used in this Label Switching Router
	 * @return the value of the new label installed 
	 */
	int installNewLabel(int outLabel, string inInterface, 
						string outInterface, int fec, int optcode);
	
	/**
	 * Find outgoing label for a packet based on the packet's Forward Equivalent Class
	 * @param fec The packet's FEC
	 * @return The outgoing label or -2 if there is no label found
	 */
	int  requestLabelforFec(int fec);
	
	/**
	 * Find the FEC based on corresponding incoming label and incoming interface
	 * @param label The incoming label
	 *        inInterface The incoming interface
	 * @return the FEC value or 0 if the FEC cannot be found
	 */
	int  findFec(int label, string inInterface);

	/** Find the outgoing interface based on the incoming interface and the outgoing label
	 * @param senderInterface The incoming interface name
	          newLabel The outgoing label

	 * @return The outgoing interface name or "X" if the outgoing interface cannot be found
	 */
	string requestOutgoingInterface(string senderInterface,int newLabel);

	/** Find the outgoing interface name based on the FEC
	 * @param fec The FEC value
	 * @return the outgoing interface name
	 */
	string requestOutgoingInterface(int fec);

	/**
	 * Find the outgoing interface name based on incoming interface,outgoing label and incoming label
	 * If the new label is different to -1 (native ip), the function is the same with 
	 * requestOutgoingInterface(string senderInterface, int newLabel)
	 * @param senderInterface The incoming interface
	          newLabel The outgoing label
			  oldLabel The incoming label
	 * @return The outgoing interface name
	 */
	string requestOutgoingInterface(string senderInterface, int newLabel, int oldLabel);

    /**
	 * Install new label based on incoming interface and incoming label
	 * @param senderInterface The incoming interface
	 *        oldLabel The incoming label
	 * @return The value of the new label installed 
	 */
	int requestNewLabel(string senderInterface,int oldLabel);

	/**
	 * Get the optcode based on incoming interface and incoming label
	 * @param senderInterface The incoming interface name
	 *        oldLabel The incoming label
	 * @return The operation code
	 */
	int getOptCode(string senderInterface, int oldLabel);

	

	/*

	//Return incomming label

	//Return outgoing label

	int ini_requestLabelforDest(IPAddressPrefix *dest);



	void updateTable(label_mapping_type *newMapping);

	//bool isMapPrefix(IPAddressPrefix *prefixAddress, IPAddress *ipAddress);

	void updateTable(int inLabel, string inInterface,

		string outInterface, int fec);

    //Return outgoing label 

	//Return outgoing interface

	//Return incomming interface 

	string requestIncommingInterface(int fec);

	*/



};


#endif

