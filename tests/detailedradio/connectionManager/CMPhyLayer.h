/* -*- mode:c++ -*- ********************************************************
 * file:        BasePhyLayer.h
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#ifndef TEST_PHY_LAYER_H
#define TEST_PHY_LAYER_H

#include <iostream>
#include "ModuleAccess.h"
#include "MACAddress.h"
#include "DetailedRadioChannelAccess.h"
#include "MACFrameBase_m.h"

/**
 * @brief
 */
class CMPhyLayer : public DetailedRadioChannelAccess
{
protected:

	MACAddress myAddr() {
		return MACAddress( findContainingNode(this)->getIndex() );
	};

	void sendDown(const MACAddress& destAddr) {
	    MACFrameBase* m = new MACFrameBase();
		m->setDestAddr(destAddr);
		m->setSrcAddr(myAddr());
		sendToChannel(m);
	}

	void assertTrue(std::string msg, bool value) {
		if (!value) {
			EV << "FAILED: ";
		} else {
		    EV << "Passed: ";
		}

		EV << "Test Node " << findContainingNode(this)->getIndex() << ": " << msg << std::endl;
	}

	void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }

public:
	//Module_Class_Members( CMPhyLayer, ConnectionManagerAccess, 0 );

	/** @brief Called every time a message arrives*/
	virtual void handleMessage( cMessage* );

	virtual void handleSelfMsg() { assertFalse("This phy layer expects no self-msg!", true); }
	virtual void handleLowerMsg(const MACAddress& /*srcAddr*/) { assertFalse("This phy layer expects no msg!", true); }

protected:
    virtual void setRadioMode(RadioMode radioMode) { }

    virtual void setRadioChannel(int radioChannel) { }

    virtual void handleMessageWhenUp(cMessage *msg) { }
};

#endif
