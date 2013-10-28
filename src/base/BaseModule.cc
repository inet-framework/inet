/* -*- mode:c++ -*- ********************************************************
 * file:        BaseModule.cc
 *
 * author:      Steffen Sroka
 *              Andreas Koepke
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
 **************************************************************************/

#include "BaseModule.h"

#include <cassert>

#include "FindModule.h"

BaseModule::BaseModule()
	: cSimpleModule()
	, cListener()
{}

BaseModule::BaseModule(unsigned stacksize)
	: cSimpleModule(stacksize)
	, cListener()
{}

/**
 * Subscription should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseModule::initialize(int stage) {
}

void BaseModule::receiveSignal(cComponent */*source*/, simsignal_t signalID, cObject *obj) {
	Enter_Method_Silent();
}

cModule* BaseModule::findHost(void)
{
	return FindModule<>::findHost(this);
}

const cModule* BaseModule::findHost(void) const
{
	return FindModule<>::findHost(this);
}
