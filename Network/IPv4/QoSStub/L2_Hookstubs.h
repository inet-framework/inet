// -*- C++ -*-
// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/*
	file: L2_Hookstubs.h
	Purpose: Header Test file for L2 queue hooks
	Responsibilities:
		define L2_DequeueHook and L2_EnqueueHook
		do nothing, just exist
		packet sink
		please replace with QoS modules later

	author: Jochen Reber
*/

#ifndef __L2_HOOK_STUBS_H__
#define __L2_HOOK_STUBS_H__

#include <omnetpp.h>

#include "basic_consts.h"

class L2_EnqueueHook: public cSimpleModule
{
private:

public:
	Module_Class_Members(L2_EnqueueHook, cSimpleModule, 0);
	
	void handleMessage(cMessage *msg);
};

class L2_DequeueHook: public cSimpleModule
{
private:

public:
	Module_Class_Members(L2_DequeueHook, cSimpleModule, 0);
	
	void handleMessage(cMessage *msg);
};

#endif

