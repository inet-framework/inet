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

#include "ILifecycle.h"
#include "BPDU.h"
#include "MACAddress.h"
#include "EtherFrame.h"
#include "MACAddressTable.h"
#include "PortFiltRSTP.h"
#include "InterfaceTable.h"
#include "IEEE8021DInterfaceData.h"

/**
 * RSTP implementation.
 */
class RSTP: public cSimpleModule, public ILifecycle
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
	bool isOperational; // for lifecycle

	cModule* Parent; /// Pointer to the parent module
	cModule* relay;

	unsigned int portCount;
	simtime_t TCWhileTime; /// TCN activation time
	bool autoEdge;	/// Automatic edge ports detection

	MACAddressTable * sw;  /// Needed for flushing.

	IInterfaceTable * ifTable;

	/*Static data. Bridge data. */
	int priority;  /// Bridge priority. It's own priority.
	MACAddress address; /// BEB MAC address

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

  protected:
	virtual void initialize(int stage);

	virtual void initInterfacedata(unsigned int portNum);

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


	virtual void updateInterfacedata(BPDUieee8021D *frame,unsigned int portNum);

	/**
	 * @brief Compares the frame with the frame this module would send through that port
	 * @return (<0 if own vector is better than frame)
	 * -4=Worse Port -3=Worse Src -2=Worse RPC -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	 */
	virtual int contestInterfacedata(BPDUieee8021D* msg,unsigned int portNum);

	/**
	 * @brief Compares the vector with the frame this module would send through that por
	 * @return (<0 if own vector is better than vect2)
	 * -4=Worse Port -3=Worse Src -2=Worse RPC -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	 */
	virtual int contestInterfacedata(unsigned int portNum);

	/**
	 * @brief If root TCWhile has not expired, sends a BPDU to the Root with TCFlag=true.
	 */

	virtual int compareInterfacedata(unsigned int portNum, BPDUieee8021D * msg,int linkCost);

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

  // for lifecycle:
  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
  protected:
    virtual void start();
    virtual void stop();
    IEEE8021DInterfaceData * getPortInterfaceData(unsigned int portNum);
};


#endif

