 /**
******************************************************
* @file RSTP.h
* @brief RSTP Protocol control
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#ifndef __A_RSTP_H
#define __A_RSTP_H

#include "BPDU.h"
#include "MACAddress.h"
#include "Delivery_m.h"
#include "Cache1Q.h"
#include "PortFilt.h"

#define UP 1
#define DOWN 0


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

	PortFilt * portfilt;

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
	  /*Dynamic data.*/

	int MaxAge;
	bool verbose;		/// Sets module verbosity
	bool testing;		/// Save Testing data
	bool up;

	cModule* Parent; /// Pointer to the parent module
	cModule* admac;

	simtime_t TCWhileTime; /// TCN activation time
	bool autoEdge;	/// Automatic edge ports detection

	Cache1Q * sw;  /// Needed for flushing.


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
	virtual void handleIncomingFrame(Delivery *frame2);

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
	 * @brief Handling UP/DOWN event
	 */
	virtual void handleUpTimeEvent(cMessage *);

	/**
	 * @brief Scheduling UP/DOWN events
	 */
	virtual void scheduleUpTimeEvent(cXMLElement * event);

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

