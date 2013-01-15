 /**
******************************************************
* @file MVRP.h
* @brief MVRP Protocol control
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date October 2010
*
*
******************************************************/
#ifndef __A_MVRP_H
#define __A_MVRP_H

#include "MVRPDU.h"
#include "MACAddress.h"
#include "RSTP.h"


struct vid_time
{
	vid VID;
	simtime_t inserted;
};							/// registered VID entry

/**
 * Contains per port VLAN registration info. MVRP protocol
 */
class PortMVRPStatus
{
public:
	std::vector <vid_time> registered;	/// Registered VLANs vids

};

/**
 * MVRP implementation.
 */
class MVRP: public cSimpleModule
{
  protected:
	//Dynamic data.
	  std::vector <PortMVRPStatus *> Puertos; /// Contains all ports MVRP information.
	  RSTP * rstpModule;	/// Points to the RSTP module. Needed when asking for the port state.

	//Static data.
	  MACAddress address; 	/// BEB MAC address
	  simtime_t interFrameTime;			/// Time between MVRPDU sending
	  simtime_t agingTime;				/// max idle time for registered entries (when it expires, entry is removed from the table)
	  bool verbose;			/// Sets the module verbosity
	  bool testing;			/// Saves testing data

  public:
	MVRP() {}
	virtual int numInitStages() const {return 3;}

	/**
	 * @brief Register or refreshes a vlan at port register
	 * @return True if refreshing. False for a new entry.
	 */
	virtual bool registerVLAN(int port,vid vlan);

	/**
	 * @brief Gets the gates where the vlan is registered. (except GateIndex)
	 * @note GateIndex not included
	 * @return True when a matching entry is found.
	 */
	virtual bool resolveVLAN(vid vlan, std::vector<int> * Gates);

	/**
	 * @brief Prints basic module information and MVRP registered info.
	 */
	virtual void printState();

  protected:
	virtual void initialize(int stage);

	/**
	 * @brief Calls the appropiate handle function
	 * @note itsMVRPDUtime message triggers a new MVRPDU
	 */
	virtual void handleMessage(cMessage *msg);

	/**
	 * @brief Updates port MVRP registered vlans with the frame info
	 */
	virtual void handleIncomingFrame(Delivery *frame);

	/**
	 * @brief Processes timing auto-messages (MVRPDU generation)
	 */
	virtual void handleMVRPDUtime(cMessage *msg);

	/**
	 * @brief Clean aged entries from MVRP registered cache.
	 * Client ports are not cleaned.
	 */
	virtual void cleanAgedEntries();

	/**
	 * @brief Saving statistics.
	 */
	virtual void finish();
};




#endif

