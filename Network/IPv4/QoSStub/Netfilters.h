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
	file: Netfilters.h
	Purpose: quick definition of IP Processing netfilters for 
                 test purposes
	author: Jochen Reber
*/

#ifndef __NETFILTERS_H__
#define __NETFILTERS_H__

#include "omnetpp.h"

#include "basic_consts.h"

// Pre Routing Netfilter
class NF_IP_PRE_ROUTING: public cSimpleModule
{
private:

public:
	Module_Class_Members(NF_IP_PRE_ROUTING, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void activity();
	
};

// Routing Netfilter
class NF_IP_FORWARD: public cSimpleModule
{
private:

public:
	Module_Class_Members (NF_IP_FORWARD, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void activity();
	
};

// Local Deliver Netfilter
class NF_IP_LOCAL_IN: public cSimpleModule
{
private:

public:
	Module_Class_Members (NF_IP_LOCAL_IN, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void activity();
	
};

// IPSend Netfilter
class NF_IP_LOCAL_OUT: public cSimpleModule
{
private:

public:
	Module_Class_Members (NF_IP_LOCAL_OUT, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void activity();
	
};

// IPOutput Netfilter
class NF_IP_POST_ROUTING: public cSimpleModule
{
private:

public:
	Module_Class_Members (NF_IP_POST_ROUTING, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void activity();
	
};

#endif

