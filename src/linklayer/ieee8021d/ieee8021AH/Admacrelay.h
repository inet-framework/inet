 /**
******************************************************
* @file Admacrelay.h
* @brief Part of the B-Component module. Switch
* Uses RSTP and MVRP modules information to determine the
* output gate.
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __A_ADMACRELAY_H
#define __A_ADMACRELAY_H

#include "MACAddress.h"
#include "EtherFrame_m.h"
#include "Cache1AHAccess.h"
#include "BPDU_m.h"
#include "RSTP.h"
#include "MVRP.h"
#include "Delivery_m.h"
#include "Relay1Q.h"



/**
 * Admacrelay module class
 */
class Admacrelay : public Relay1Q
{
	protected:
		Cache1AH *cache;   /// SwTable module pointer.
	public:
		Admacrelay();
		virtual ~Admacrelay();
	protected:
	    virtual void initialize(int stage);
	    virtual int numInitStages() const {return 3;}



	/**
	 * 802.1ah handler. Determines correct output gate and calls relayMsg.
	 */
	    virtual void handleEtherFrame(EthernetIIFrame *frame);  /// 802.1ah handler.

	/**
	 * Does the relaying task.
	 */
	    virtual void relayMsg(cMessage * msg,std::vector <int> outputPorts);

	/**
	 * @brief Checks mac and shows if it is a Backbone Service Instance Group address.
	 */
	    virtual bool isISidBroadcast(MACAddress mac, int ISid);
};

#endif

