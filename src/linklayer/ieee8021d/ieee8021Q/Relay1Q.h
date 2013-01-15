 /**
******************************************************
* @file Relay1Q.h
* @brief Part of the ieee802.Q bridge. Relay
* Uses RSTP and MVRP modules information to deliver
* messages through the correct gate
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/
#ifndef __A_RELAY1Q_H
#define __A_RELAY1Q_H

#include "MACAddress.h"
#include "EtherFrame_m.h"
#include "BPDU.h"
#include "RSTP.h"
#include "MVRP.h"
#include "Delivery_m.h"


/**
 * Relay1Q module class
 */
class Relay1Q : public cSimpleModule
{
protected:
  Cache1Q *cache;   /// SwTable module pointer. Updated every time the admcarelay module is accessed.
  RSTP * rstpModule; /// RSTP module pointer. Updated every time the admcarelay module is accessed.
  MVRP * mvrpModule; /// MVRP module pointer. Updated every time the admcarelay module is accessed.
  bool verbose;  /// It sets module verbosity
public:
	Relay1Q();
	~Relay1Q();


protected:
	MACAddress address;
	virtual void initialize(int stage);
    virtual int numInitStages() const {return 3;}
	virtual void handleMessage(cMessage *msg);  ///Basic handling. @see handleIncomingFrame
	virtual void handleIncomingFrame(BPDUieee8021D *frame);				/// BPDU handler. Delivers BPDUs to every Designated port or to the RSTP module.
	virtual void handleIncomingFrame(MVRPDU *frame);  			/// MVRPDU handler. Delivers MVRPDUs based on the PortIndex value and the source address.
	virtual void handleIncomingFrame(Delivery *frame);
	virtual void handleIncomingFrame(cMessage *msg);		//Generic data frame handling
	virtual void handleEtherFrame(EthernetIIFrame *frame);  /// 802.1Q handler. Does the relaying task
	virtual void finish();
	virtual void relayMsg(cMessage * msg,std::vector <int> outputPorts);


};

#endif

