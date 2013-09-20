//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __A_RSTP_H
#define __A_RSTP_H

#include "BPDU.h"
#include "MACAddress.h"
#include "Delivery.h"
#include "MACAddressTable.h"
#include "PortFiltRSTP.h"
#include "InterfaceTable.h"


enum PortStateT
{
	DISCARDING,
	LEARNING,
	FORWARDING
};	/// Port state. Current behavior

enum PortRoleT
{
	ALTERNATE_PORT,
	NOTASIGNED,
	DISABLED,
	DESIGNATED,
	BACKUP,
	ROOT
}; /// Port role within the current topology


/**
 * @brief RSTP information for a particular port
 */
class RSTPVector
{
public:
	int RootPriority;
	MACAddress RootMAC;
	int RootPathCost;
	int Age;
	int srcPriority;
	MACAddress srcAddress;
	int srcPortPriority;
	int srcPort;
	int arrivalPort;


	/**
	 * @brief Gets the RSTP vector i to the initial case. (auto proposed)
	 */
	virtual void init(int priority, MACAddress address);


	/**
	 * @brief Compares a RSTPVector with BPDU contained info.
	 * @return (<0 if vector better than frame)
	 * -4=Worse Port -3=Worse Src -2=Worse RPC -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	 */
	virtual int compareRstpVector(BPDUieee8021D * frame,int PortCost);

	/**
	 * @brief Compares two RSTPVector.
	 * @return <0 if vect1 is better than vect 2. >0 if vect1 worse than vect2. 0 if they are similar
	 */
	virtual int compareRstpVector(RSTPVector vect2);
};

class PortStatus
{
public:
    bool Edge;		 // It indicates that this is a client port.
	int Flushed;	 // Counts how many times this port cache has been flushed during simulation

	PortRoleT PortRole;
	PortStateT PortState;
	int LostBPDU;			//Lost BPDU counter. Used to detect topology changes.
	RSTPVector PortRstpVector;	//best received RSTPvector in that port.
	int PortCost;			//Cost for that port.
	int PortPriority;		//Priority for that port.
	simtime_t TCWhile;		//This port will send TC=true until this time has been overtaken.

	PortFiltRSTP * portfilt;

	PortStatus();
	virtual ~PortStatus(){}
	virtual void updatePortVector(BPDUieee8021D *frame,int arrival);
}; /// Global per port state info

/**
 * RSTP implementation.
 */
class RSTP: public cSimpleModule
{
  protected:
    /* kind codes for self messages */
    enum SelfKinds
    {
        SELF_HELLOTIME = 1,
        SELF_UPGRADE,
        SELF_TIMETODESIGNATE
    };

	  /*Dynamic data.*/

	int MaxAge;
	bool verbose;		/// Sets module verbosity
	bool testing;		/// Save Testing data

	cModule* Parent; /// Pointer to the parent module
	cModule* admac;

	simtime_t TCWhileTime; /// TCN activation time
	bool autoEdge;	/// Automatic edge ports detection

	MACAddressTable * sw;  /// Needed for flushing.

	IInterfaceTable * ifTable;

	/*Static data. Bridge data. */
	int priority;  /// Bridge priority. It's own priority.
	MACAddress address; /// BEB MAC address

	std::vector <PortStatus> Puertos;	/// Vector with all port status
	simtime_t hellotime;	/// Time between hello BPDUs
	simtime_t forwardDelay;	/// After that a discarding port switches to learning and so on if it is designated
	simtime_t migrateTime;  /// After that, a not asigned port becomes designated

	cMessage* helloM;
	cMessage* forwardM;
	cMessage* migrateM;

  public:
	RSTP();
	virtual ~RSTP();
	virtual int numInitStages() const {return 2;}

	/**
	* @brief Obtains the port role
	* @param index Port index
	* @return PortRole
	*/
	virtual PortRoleT getPortRole(int index);
	/**
	* @brief Obtains the port state
	* @param index Port index
	* @return PortState
	*/
	virtual PortStateT getPortState(int index);
	/**
	* @brief Determines if a port is connected to a client network
	* @param index Port index
	* @return True if it is connected to a client network
	*/
	virtual bool isEdge(int index);

	/**
	* @brief Get BEB address.
	*/
	virtual MACAddress getAddress();

  protected:
	virtual void initialize(int stage);

	/**
	 * @brief initialize (Puertos) RSTP dynamic information
	 */
	virtual void initPorts();

	/**
	 * @brief Gets the best alternate port
	 * @return Best alternate gate index
	 */
	virtual int getBestAlternate();

	/**
	 * @brief Adds effects to be represented by Tkenv. Root links colored green. Port state.
	 */
	virtual void colorRootPorts();

	/**
	 * @brief Sends BPDUs through all ports, if they are required
	 */
	virtual void sendBPDUs();

	/**
	 * @brief Sends BPDU through port
	 */
	virtual void sendBPDU(int port);


	/**
	 * @brief General processing
	 */
	virtual void handleMessage(cMessage *msg);
	/**
	 * @brief BPDU processing.
	 * Updates RSTP vectors information. Handles port role changes.
	 */
	virtual void handleIncomingFrame(BPDUieee8021D *frame);

	/**
	 * @brief Savin statistics
	 */
	virtual void finish();
	/**
	 * @brief Prints current data base info
	 */
	virtual void printState();
	/**
	 * @brief Obtain the root gate index
	 * @return Gate index
	 */
	virtual int getRootIndex();
	/**
	 * @brief Obtains the root RSTP vector.
	 * Root gate RSTP vector
	 */
	virtual RSTPVector getRootRstpVector();

	/**
	 * @brief Compares the frame with the frame this module would send through that port
	 * @return (<0 if own vector is better than frame)
	 * -4=Worse Port -3=Worse Src -2=Worse RPC -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	 */
	virtual int contestRstpVector(BPDUieee8021D *frame, int arrival);

	/**
	 * @brief Compares the vector with the frame this module would send through that por
	 * @return (<0 if own vector is better than vect2)
	 * -4=Worse Port -3=Worse Src -2=Worse RPC -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	 */
	virtual int contestRstpVector(RSTPVector vect2,int v2Port);

	/**
	 * @brief If root TCWhile has not expired, sends a BPDU to the Root with TCFlag=true.
	 */
	virtual void sendTCNtoRoot();

	/**
	 * @brief HelloTime event handling.
	 */
	virtual void handleHelloTime(cMessage *);

	/**
	 * @brief Upgrade event handling. (Every forwardDelay)
	 */
	virtual void handleUpgrade(cMessage *);

	/**
	 * @brief Migration to designated. (Every migrateTime)
	 */
	virtual void handleMigrate(cMessage *);

	/**
	 * @brief Checks the frame TC flag.
	 * Sets TCWhile if the port was forwarding and the flag is true.
	 */
	virtual void checkTC(BPDUieee8021D * frame, int arrival);

	/**
	 * @brief Handles the switch to backup in one of the ports
	 */
	virtual void handleBK(BPDUieee8021D * frame, int arrival);
};


#endif

